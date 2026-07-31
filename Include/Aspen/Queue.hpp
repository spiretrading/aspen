#ifndef ASPEN_QUEUE_HPP
#define ASPEN_QUEUE_HPP
#include <concepts>
#include <cstdint>
#include <deque>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <utility>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * A reactor that evaluates to the values pushed to an internal queue.
   * @param <T> The type of values to queue.
   */
  template<typename T>
  class Queue {
    public:

      /** The type of values to queue. */
      using Type = T;

      /** Constructs an empty Queue. */
      Queue();

      Queue(Queue&& queue);

      /**
       * Pushes a value to the queue.
       * @param value The value to push.
       */
      void push(Type value);

      /** Brings this reactor to a completion state. */
      void set_complete();

      /**
       * Pushes a value and brings this reactor to a completion state.
       * @param value The value to push.
       */
      void set_complete(Type value);

      /**
       * Sets an exception and brings this reactor to a completion state.
       * @param exception The exception to throw.
       */
      void set_complete(std::exception_ptr exception);

      /**
       * Brings this reactor to a completion state by throwing an exception.
       * @param exception The exception to throw.
       */
      template<std::derived_from<std::exception> E>
      void set_complete(const E& exception);

      State commit(std::uint64_t sequence) noexcept;
      eval_result_t<Type> eval() const;
      Queue& operator =(Queue&& queue);

    private:
      mutable std::mutex m_mutex;
      bool m_is_complete;
      bool m_has_commit;
      std::deque<Type> m_entries;
      std::exception_ptr m_exception;
      CommitFlag* m_flag;

      void update(auto&& f);
  };

  template<typename T>
  Queue<T>::Queue()
    : m_is_complete(false),
      m_has_commit(false),
      m_flag(nullptr) {}

  template<typename T>
  Queue<T>::Queue(Queue&& queue) {
    auto lock = std::lock_guard(queue.m_mutex);
    m_is_complete = queue.m_is_complete;
    m_has_commit = queue.m_has_commit;
    m_entries = std::move(queue.m_entries);
    m_exception = std::move(queue.m_exception);
    m_flag = nullptr;
  }

  template<typename T>
  void Queue<T>::push(Type value) {
    update([&] {
      m_entries.emplace_back(std::move(value));
    });
  }

  template<typename T>
  void Queue<T>::set_complete() {
    update([&] {
      m_is_complete = true;
    });
  }

  template<typename T>
  void Queue<T>::set_complete(Type value) {
    update([&] {
      m_is_complete = true;
      m_entries.emplace_back(std::move(value));
    });
  }

  template<typename T>
  void Queue<T>::set_complete(std::exception_ptr exception) {
    update([&] {
      m_is_complete = true;
      m_exception = std::move(exception);
    });
  }

  template<typename T>
  template<std::derived_from<std::exception> E>
  void Queue<T>::set_complete(const E& exception) {
    set_complete(std::make_exception_ptr(exception));
  }

  template<typename T>
  State Queue<T>::commit(std::uint64_t sequence) noexcept {
    auto lock = std::lock_guard(m_mutex);
    m_flag = CommitFlag::get_current();
    if(m_entries.size() > 1 || (m_entries.size() == 1 && !m_has_commit)) {
      if(m_has_commit) {
        m_entries.pop_front();
      } else {
        m_has_commit = true;
      }
      if(m_entries.size() > 1 || m_exception) {
        return State::CONTINUE_EVALUATED;
      } else if(m_is_complete) {
        return State::COMPLETE_EVALUATED;
      }
      return State::EVALUATED;
    } else if(m_exception) {
      m_entries.clear();
      return State::COMPLETE_EVALUATED;
    } else if(m_is_complete) {
      return State::COMPLETE;
    }
    return State::NONE;
  }

  template<typename T>
  eval_result_t<typename Queue<T>::Type> Queue<T>::eval() const {
    auto lock = std::lock_guard(m_mutex);
    if(m_entries.empty()) {
      if(!m_exception) {
        throw std::runtime_error("Uninitialized.");
      }
      std::rethrow_exception(m_exception);
    }
    return m_entries.front();
  }

  template<typename T>
  Queue<T>& Queue<T>::operator =(Queue&& queue) {
    if(this == &queue) {
      return *this;
    }
    auto flag = [&] {
      auto lock = std::scoped_lock(m_mutex, queue.m_mutex);
      m_is_complete = queue.m_is_complete;
      m_has_commit = queue.m_has_commit;
      m_entries = std::move(queue.m_entries);
      m_exception = std::move(queue.m_exception);
      return m_flag;
    }();
    if(flag) {
      flag->raise();
    }
    return *this;
  }

  template<typename T>
  void Queue<T>::update(auto&& f) {
    auto flag = [&] () -> CommitFlag* {
      auto lock = std::lock_guard(m_mutex);
      if(m_is_complete) {
        return nullptr;
      }
      f();
      return m_flag;
    }();
    if(flag) {
      flag->raise();
    }
  }
}

#endif

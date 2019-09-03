#ifndef ASPEN_QUEUE_HPP
#define ASPEN_QUEUE_HPP
#include <deque>
#include <exception>
#include <mutex>
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"
#include "Aspen/Trigger.hpp"

namespace Aspen {

  /**
   * A reactor that evaluates to the values pushed to an internal queue.
   * @param <T> The type of values to queue.
   */
  template<typename T>
  class Queue {
    public:
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
      template<typename E>
      void set_complete(const E& exception);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const;

      Queue& operator =(Queue&& queue);

    private:
      std::mutex m_mutex;
      bool m_is_complete;
      bool m_has_commit;
      std::deque<Type> m_entries;
      std::exception_ptr m_exception;
      Trigger* m_trigger;
  };

  template<typename T>
  Queue<T>::Queue()
    : m_is_complete(false),
      m_has_commit(false),
      m_trigger(nullptr) {}

  template<typename T>
  Queue<T>::Queue(Queue&& queue) {
    auto lock = std::lock_guard(queue.m_mutex);
    m_is_complete = std::move(queue.m_is_complete);
    m_has_commit = std::move(queue.m_has_commit);
    m_entries = std::move(queue.m_entries);
    m_exception = std::move(queue.m_exception);
    m_trigger = std::move(queue.m_trigger);
  }

  template<typename T>
  void Queue<T>::push(Type value) {
    auto lock = std::lock_guard(m_mutex);
    m_entries.emplace_back(std::move(value));
    if(m_trigger != nullptr) {
      m_trigger->signal();
    }
  }

  template<typename T>
  void Queue<T>::set_complete() {
    auto lock = std::lock_guard(m_mutex);
    m_is_complete = true;
    if(m_trigger != nullptr) {
      m_trigger->signal();
    }
  }

  template<typename T>
  void Queue<T>::set_complete(Type value) {
    auto lock = std::lock_guard(m_mutex);
    m_is_complete = true;
    m_entries.emplace_back(std::move(value));
    if(m_trigger != nullptr) {
      m_trigger->signal();
    }
  }

  template<typename T>
  void Queue<T>::set_complete(std::exception_ptr exception) {
    auto lock = std::lock_guard(m_mutex);
    m_exception = std::move(exception);
    if(m_trigger != nullptr) {
      m_trigger->signal();
    }
  }

  template<typename T>
  template<typename E>
  void Queue<T>::set_complete(const E& exception) {
    set_complete(std::make_exception_ptr(exception));
  }

  template<typename T>
  State Queue<T>::commit(int sequence) noexcept {
    auto lock = std::lock_guard(m_mutex);
    if(m_trigger == nullptr) {
      m_trigger = Trigger::get_trigger();
    }
    auto state = [&] {
      if(m_entries.size() > 1 || m_entries.size() == 1 && !m_has_commit) {
        if(m_has_commit) {
          m_entries.pop_front();
        } else {
          m_has_commit = true;
        }
        if(m_entries.size() > 1 || m_exception != nullptr) {
          return State::CONTINUE_EVALUATED;
        } else if(m_is_complete) {
          return State::COMPLETE_EVALUATED;
        } else {
          return State::EVALUATED;
        }
      } else if(m_exception != nullptr) {
        m_entries.clear();
        return State::COMPLETE_EVALUATED;
      } else if(m_is_complete) {
        return State::COMPLETE;
      } else {
        return State::NONE;
      }
    }();
    return state;
  }

  template<typename T>
  eval_result_t<typename Queue<T>::Type> Queue<T>::eval() const {
    if(m_entries.empty()) {
      std::rethrow_exception(m_exception);
    }
    return m_entries.front();
  }

  template<typename T>
  Queue<T>& Queue<T>::operator =(Queue&& queue) {
    {
      auto lock = std::lock_guard(queue.m_mutex);
      m_is_complete = std::move(queue.m_is_complete);
      m_has_commit = std::move(queue.m_has_commit);
      m_entries = std::move(queue.m_entries);
      m_exception = std::move(queue.m_exception);
      m_trigger = std::move(queue.m_trigger);
    }
    return *this;
  }
}

#endif

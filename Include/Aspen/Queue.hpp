#ifndef ASPEN_QUEUE_HPP
#define ASPEN_QUEUE_HPP
#include <deque>
#include <exception>
#include <mutex>
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"
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

      /**
       * Pushes a value to the queue.
       * @param value The value to push.
       */
      void push(Type value);

      /** Brings this reactor to a completion state. */
      void set_complete();

      /**
       * Brings this reactor to a completion state by throwing an exception.
       * @param exception The exception to throw.
       */
      void set_complete(std::exception_ptr exception);

      /**
       * Brings this reactor to a completion state by throwing an exception.
       * @param exception The exception to throw.
       */
      template<typename E>
      void set_complete(const E& exception);

      State commit(int sequence);

      const Type& eval() const;

    private:
      struct Entry {
        Type m_value;
        int m_sequence;
      };
      std::mutex m_mutex;
      std::deque<Entry> m_entries;
      std::exception_ptr m_exception;
      Maybe<Type> m_value;
      int m_completion_sequence;
      Trigger* m_trigger;
      State m_state;
      int m_previous_sequence;
  };

  template<typename T>
  Queue<T>::Queue()
      : m_completion_sequence(-1),
        m_trigger(nullptr),
        m_state(State::UNINITIALIZED),
        m_previous_sequence(-1) {}

  template<typename T>
  void Queue<T>::push(Type value) {
    auto lock = std::lock_guard(m_mutex);
    m_entries.emplace_back(std::move(value), 0);
    m_trigger->signal(m_entries.back().m_sequence);
  }

  template<typename T>
  void Queue<T>::set_complete() {
    auto lock = std::lock_guard(m_mutex);
    m_trigger->signal(m_completion_sequence);
  }

  template<typename T>
  void Queue<T>::set_complete(std::exception_ptr exception) {
    m_exception = std::move(exception);
    auto lock = std::lock_guard(m_mutex);
    m_trigger->signal(m_completion_sequence);
  }

  template<typename T>
  template<typename E>
  void Queue<T>::set_complete(const E& exception) {
    set_complete(std::make_exception_ptr(exception));
  }

  template<typename T>
  State Queue<T>::commit(int sequence) {
    if(is_complete(m_state) || sequence == m_previous_sequence) {
      return m_state;
    } else if(m_state == State::UNINITIALIZED) {
      m_trigger = &Trigger::get_trigger();
    }
    auto lock = std::lock_guard(m_mutex);
    if(sequence == m_completion_sequence) {
      if(m_exception == nullptr) {
        m_state = State::COMPLETE_EMPTY;
      } else {
        m_value = std::move(m_exception);
        m_state = State::COMPLETE_EVALUATED;
      }
    } else if(!m_entries.empty() && sequence == m_entries.front().m_sequence) {
      m_value = std::move(m_entries.front().m_value);
      m_entries.pop_front();
      m_state = State::EVALUATED;
    } else {
      m_state = State::NONE;
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename T>
  const typename Queue<T>::Type& Queue<T>::eval() const {
    return m_value;
  }
}

#endif

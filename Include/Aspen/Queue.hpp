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

      State commit(int sequence);

      const Type& eval() const;

    private:
      std::mutex m_mutex;
      bool m_is_complete;
      std::deque<Type> m_entries;
      std::exception_ptr m_exception;
      Maybe<Type> m_value;
      Trigger* m_trigger;
      State m_state;
      int m_previous_sequence;
      bool m_had_evaluation;
  };

  template<typename T>
  Queue<T>::Queue()
    : m_is_complete(false),
      m_trigger(nullptr),
      m_state(State::NONE),
      m_previous_sequence(-1),
      m_had_evaluation(false) {}

  template<typename T>
  void Queue<T>::push(Type value) {
    auto lock = std::lock_guard(m_mutex);
    m_entries.emplace_back(std::move(value));
    m_had_evaluation = true;
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
    m_had_evaluation = true;
    if(m_trigger != nullptr) {
      m_trigger->signal();
    }
  }

  template<typename T>
  void Queue<T>::set_complete(std::exception_ptr exception) {
    auto lock = std::lock_guard(m_mutex);
    m_exception = std::move(exception);
    m_had_evaluation = true;
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
  State Queue<T>::commit(int sequence) {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    auto lock = std::lock_guard(m_mutex);
    if(m_previous_sequence == -1) {
      m_trigger = Trigger::get_trigger();
    }
    if(!m_entries.empty()) {
      m_value = std::move(m_entries.front());
      m_entries.pop_front();
      m_state = State::EVALUATED;
      if(!m_entries.empty() || m_exception != nullptr) {
        m_state = combine(m_state, State::CONTINUE);
      } else if(m_is_complete) {
        m_state = combine(m_state, State::COMPLETE);
      }
    } else if(m_exception != nullptr) {
      m_value = std::move(m_exception);
      m_state = State::COMPLETE_EVALUATED;
    } else if(m_is_complete) {
      m_state = State::COMPLETE;
      if(!m_had_evaluation) {
        m_state = combine(m_state, State::EMPTY);
      }
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

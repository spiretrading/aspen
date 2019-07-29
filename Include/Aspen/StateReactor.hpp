#ifndef ASPEN_STATE_REACTOR_HPP
#define ASPEN_STATE_REACTOR_HPP
#include <type_traits>
#include "Aspen/LocalPtr.hpp"
#include "Aspen/State.hpp"

namespace Aspen {

  /**
   * A reactor that evaluates to another reactor's State.
   * @param <R> The type of reactor to monitor.
   */
  template<typename R>
  class StateReactor {
    public:
      using Type = State;

      /**
       * Constructs a StateReactor.
       * @param reactor The reactor to monitor.
       */
      template<typename RF>
      explicit StateReactor(RF&& reactor);

      State commit(int sequence) noexcept;

      Type eval() const noexcept;

    private:
      try_ptr_t<R> m_reactor;
      State m_value;
      State m_state;
  };

  template<typename R>
  StateReactor(R&&) -> StateReactor<std::decay_t<R>>;

  template<typename R>
  template<typename RF>
  StateReactor<R>::StateReactor(RF&& reactor)
    : m_reactor(std::forward<RF>(reactor)),
      m_value(State::EVALUATED),
      m_state(State::EMPTY) {}

  template<typename R>
  State StateReactor<R>::commit(int sequence) noexcept {
    if(is_complete(m_state)) {
      return m_state;
    }
    auto value = m_reactor->commit(sequence);
    if(is_complete(value)) {
      m_state = State::COMPLETE_EVALUATED;
    } else if(value == State::NONE && m_value == State::NONE ||
        value == State::EMPTY && m_value == State::EMPTY) {
      m_state = State::NONE;
    } else {
      m_state = State::EVALUATED;
    }
    m_value = value;
    if(has_continuation(m_value)) {
      m_state = combine(m_state, State::CONTINUE);
    }
    return m_state;
  }

  template<typename R>
  typename StateReactor<R>::Type StateReactor<R>::eval() const noexcept {
    return m_value;
  }
}

#endif

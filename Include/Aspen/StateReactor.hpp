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
      StateReactor(RF&& reactor);

      State commit(int sequence);

      Type eval() const;

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
      m_state(State::NONE) {}

  template<typename R>
  State StateReactor<R>::commit(int sequence) {
    if(is_complete(m_state)) {
      return m_state;
    }
    m_value = m_reactor->commit(sequence);
    if(is_complete(m_value)) {
      m_state = State::COMPLETE_EVALUATED;
    } else {
      m_state = State::EVALUATED;
    }
    return m_state;
  }

  template<typename R>
  typename StateReactor<R>::Type StateReactor<R>::eval() const {
    return m_value;
  }
}

#endif
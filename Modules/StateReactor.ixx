export module Aspen:StateReactor;

import <utility>;
import :State;
import :Traits;

export namespace Aspen {

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
      template<typename RF, typename = std::enable_if_t<
        !std::is_base_of_v<StateReactor, std::decay_t<RF>>>>
      explicit StateReactor(RF&& reactor);

      State commit(int sequence) noexcept;

      Type eval() const noexcept;

    private:
      R m_reactor;
      State m_value;
  };

  template<typename R, typename = std::enable_if_t<
    !std::is_base_of_v<StateReactor<to_reactor_t<R>>, std::decay_t<R>>>>
  StateReactor(R&&) -> StateReactor<to_reactor_t<R>>;

  template<typename R>
  template<typename RF, typename>
  StateReactor<R>::StateReactor(RF&& reactor)
    : m_reactor(std::forward<RF>(reactor)),
      m_value(State::EVALUATED) {}

  template<typename R>
  State StateReactor<R>::commit(int sequence) noexcept {
    auto value = m_reactor.commit(sequence);
    auto state = [&] {
      if(is_complete(value)) {
        return State::COMPLETE_EVALUATED;
      } else if(value == State::NONE && m_value == State::NONE) {
        return State::NONE;
      } else {
        return State::EVALUATED;
      }
    }();
    m_value = value;
    if(has_continuation(m_value)) {
      state = combine(state, State::CONTINUE);
    }
    return state;
  }

  template<typename R>
  typename StateReactor<R>::Type StateReactor<R>::eval() const noexcept {
    return m_value;
  }
}

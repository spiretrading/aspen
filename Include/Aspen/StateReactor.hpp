#ifndef ASPEN_STATE_REACTOR_HPP
#define ASPEN_STATE_REACTOR_HPP
#include <concepts>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * A reactor that evaluates to another reactor's State.
   * @param <R> The type of reactor to monitor.
   */
  template<IsReactor R>
  class StateReactor {
    public:

      /** The type to evaluate to. */
      using Type = State;

      /**
       * Constructs a StateReactor.
       * @param reactor The reactor to monitor.
       */
      template<typename RF> requires(
        !std::derived_from<std::remove_cvref_t<RF>, StateReactor<R>>) &&
        std::constructible_from<R, RF>
      explicit StateReactor(RF&& reactor) noexcept(
        std::is_nothrow_constructible_v<R, RF>);

      State commit(std::uint64_t sequence) noexcept;
      Type eval() const noexcept;

    private:
      std::optional<R> m_reactor;
      State m_value;
  };

  template<typename R> requires(
    !std::derived_from<std::remove_cvref_t<R>, StateReactor<to_reactor_t<R>>>)
  StateReactor(R&&) -> StateReactor<to_reactor_t<R>>;

  template<IsReactor R>
  template<typename RF> requires(
    !std::derived_from<std::remove_cvref_t<RF>, StateReactor<R>>) &&
    std::constructible_from<R, RF>
  StateReactor<R>::StateReactor(RF&& reactor) noexcept(
    std::is_nothrow_constructible_v<R, RF>)
    : m_reactor(std::forward<RF>(reactor)),
      m_value(State::EVALUATED) {}

  template<IsReactor R>
  State StateReactor<R>::commit(std::uint64_t sequence) noexcept {
    auto value = m_reactor->commit(sequence);
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
    if(is_complete(value)) {
      m_reactor = std::nullopt;
    }
    if(has_continuation(m_value)) {
      state = combine(state, State::CONTINUE);
    }
    return state;
  }

  template<IsReactor R>
  typename StateReactor<R>::Type StateReactor<R>::eval() const noexcept {
    return m_value;
  }
}

#endif

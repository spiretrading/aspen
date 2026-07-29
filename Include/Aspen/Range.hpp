#ifndef ASPEN_RANGE_HPP
#define ASPEN_RANGE_HPP
#include <algorithm>
#include <optional>
#include <utility>
#include "Aspen/Constant.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/State.hpp"
#include "Aspen/StateReactor.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Makes a reactor that counts from a starting value to an end value
   * (exclusive).
   * @param start The first value to evaluate to.
   * @param stop The value to stop evaluating at (exclusive).
   * @param step The value to increment the evaluation by.
   * @return A reactor evaluating to each value from <i>start</i> to <i>stop</i>
   *         in turn.
   */
  template<typename S, typename E, typename T> requires
    IsReactor<to_reactor_t<S>> && IsReactor<to_reactor_t<E>> &&
    IsReactor<to_reactor_t<T>>
  auto range(S&& start, E&& stop, T&& step) {
    using Type = reactor_result_t<S>;
    auto start_reactor = Shared(std::forward<S>(start));
    auto start_updates = StateReactor(start_reactor);
    auto stop_reactor = Shared(std::forward<E>(stop));
    auto stop_updates = StateReactor(stop_reactor);
    auto step_reactor = Shared(std::forward<T>(step));
    auto step_updates = StateReactor(step_reactor);
    return lift([value = std::optional<Type>()] (
        const reactor_result_t<S>& start, State start_state,
        const reactor_result_t<E>& end, State end_state,
        const reactor_result_t<T>& step, State step_state) mutable noexcept {
      auto current = [&] {
        if(!value) {
          return start;
        }
        auto increment = *value + step;
        return std::max(start, increment);
      }();
      if(current >= end) {
        if(is_complete(end_state)) {
          return FunctionEvaluation<Type>(State::COMPLETE);
        }
        return FunctionEvaluation<Type>(State::NONE);
      }
      value = current;
      if(*value + step >= end) {
        if(is_complete(end_state)) {
          return FunctionEvaluation(*value, State::COMPLETE);
        }
        return FunctionEvaluation(*value);
      }
      return FunctionEvaluation(*value, State::CONTINUE_EVALUATED);
    }, std::move(start_reactor), std::move(start_updates),
      std::move(stop_reactor), std::move(stop_updates), std::move(step_reactor),
      std::move(step_updates));
  }

  /**
   * Makes a reactor that counts from a starting value to an end value
   * (exclusive) in steps of one.
   * @param start The first value to evaluate to.
   * @param stop The value to stop evaluating at (exclusive).
   * @return A reactor evaluating to each value from <i>start</i> to
   *         <i>stop</i> in turn.
   */
  template<typename S, typename E> requires IsReactor<to_reactor_t<S>> &&
    IsReactor<to_reactor_t<E>>
  auto range(S&& start, E&& stop) {
    return range(std::forward<S>(start), std::forward<E>(stop), 1);
  }
}

#endif

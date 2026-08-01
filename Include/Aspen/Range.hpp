#ifndef ASPEN_RANGE_HPP
#define ASPEN_RANGE_HPP
#include <algorithm>
#include <optional>
#include <type_traits>
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
    IsReactor<to_reactor_t<T>> &&
    std::same_as<reactor_result_t<S>, reactor_result_t<E>> &&
    std::same_as<reactor_result_t<S>, reactor_result_t<T>>
  auto range(S&& start, E&& stop, T&& step) {
    using Type = reactor_result_t<S>;
    auto stop_reactor = Shared(std::forward<E>(stop));
    auto stop_updates = StateReactor(stop_reactor);
    return lift([value = std::optional<Type>()] (const Type& start,
        const Type& end, State end_state, const Type& step) mutable noexcept(
          std::is_nothrow_copy_constructible_v<Type> && noexcept(
            std::declval<const Type&>() + std::declval<const Type&>())) {
      auto current = [&] () -> Type {
        if(!value) {
          return start;
        }
        auto increment = *value + step;
        return std::max<Type>(start, increment);
      }();
      if(value && current <= *value) {
        return FunctionEvaluation<Type>(State::NONE);
      }
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
    }, std::forward<S>(start), std::move(stop_reactor),
      std::move(stop_updates), std::forward<T>(step));
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
    return range(std::forward<S>(start), std::forward<E>(stop),
      static_cast<reactor_result_t<S>>(1));
  }
}

#endif

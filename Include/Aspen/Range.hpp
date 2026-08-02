#ifndef ASPEN_RANGE_HPP
#define ASPEN_RANGE_HPP
#include <algorithm>
#include <concepts>
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
   * Makes a reactor that counts from a starting value to a stopping value
   * (exclusive).
   * @param start The first value to evaluate to.
   * @param stop The value to stop evaluating at (exclusive).
   * @param step The value to increment the evaluation by, which must be
   *        positive for the count to advance.
   * @return A reactor evaluating to each value from <i>start</i> to <i>stop</i>
   *         in turn.
   */
  template<typename Start, typename Stop, typename Step> requires
    IsReactor<to_reactor_t<Start>> && IsReactor<to_reactor_t<Stop>> &&
    IsReactor<to_reactor_t<Step>> &&
    std::same_as<reactor_result_t<Start>, reactor_result_t<Stop>> &&
    std::same_as<reactor_result_t<Start>, reactor_result_t<Step>>
  auto range(Start&& start, Stop&& stop, Step&& step) {
    using Type = reactor_result_t<Start>;
    auto stop_reactor = Shared(std::forward<Stop>(stop));
    auto stop_updates = StateReactor(stop_reactor);
    return lift([value = std::optional<Type>()] (const Type& start,
        const Type& stop, State stop_state, const Type& step) mutable noexcept(
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
      if(current >= stop) {
        if(is_complete(stop_state)) {
          return FunctionEvaluation<Type>(State::COMPLETE);
        }
        return FunctionEvaluation<Type>(State::NONE);
      }
      value = current;
      if(*value + step >= stop) {
        if(is_complete(stop_state)) {
          return FunctionEvaluation(*value, State::COMPLETE);
        }
        return FunctionEvaluation(*value);
      }
      return FunctionEvaluation(*value, State::CONTINUE_EVALUATED);
    }, std::forward<Start>(start), std::move(stop_reactor),
      std::move(stop_updates), std::forward<Step>(step));
  }

  /**
   * Makes a reactor that counts from a starting value to a stopping value
   * (exclusive) in steps of one.
   * @param start The first value to evaluate to.
   * @param stop The value to stop evaluating at (exclusive).
   * @return A reactor evaluating to each value from <i>start</i> to
   *         <i>stop</i> in turn.
   */
  template<typename Start, typename Stop> requires
    IsReactor<to_reactor_t<Start>> && IsReactor<to_reactor_t<Stop>> &&
    std::same_as<reactor_result_t<Start>, reactor_result_t<Stop>>
  auto range(Start&& start, Stop&& stop) {
    return range(std::forward<Start>(start), std::forward<Stop>(stop),
      static_cast<reactor_result_t<Start>>(1));
  }
}

#endif

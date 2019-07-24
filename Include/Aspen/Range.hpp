#ifndef ASPEN_RANGE_HPP
#define ASPEN_RANGE_HPP
#include <algorithm>
#include <optional>
#include <utility>
#include "Aspen/Constant.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"
#include "Aspen/StateReactor.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Makes a reactor that counts from a starting value to an end value
   * (inclusive).
   * @param start The first value to evaluate to.
   * @param stop The value to stop evaluating at (exclusive).
   * @param step The value to increment the evaluation by.
   */
  template<typename S, typename E, typename T>
  auto range(S&& start, E&& stop, T&& step) {
    using Type = reactor_result_t<S>;
    auto start_reactor = make_ptr(std::forward<S>(start));
    auto start_updates = StateReactor(&*start_reactor);
    auto stop_reactor = make_ptr(std::forward<E>(stop));
    auto stop_updates = StateReactor(&*stop_reactor);
    auto step_reactor = make_ptr(std::forward<T>(step));
    auto step_updates = StateReactor(&*step_reactor);
    return lift(
      [value = std::optional<Type>()](const auto& start, State start_state,
          const auto& end, State end_state, const auto& step,
          State step_state) mutable {
        auto c = [&] {
          if(!value.has_value()) {
            return start.get();
          } else {
            auto increment = *value + step.get();
            return std::max(start.get(), increment);
          }
        }();
        if(c >= end.get()) {
          if(is_complete(end_state)) {
            return FunctionEvaluation<Type>(State::COMPLETE);
          }
          return FunctionEvaluation<Type>(State::NONE);
        }
        value = c;
        if(*value + step.get() >= end.get()) {
          if(is_complete(end_state)) {
            return FunctionEvaluation(*value, State::COMPLETE);
          } else {
            return FunctionEvaluation(*value);
          }
        }
        return FunctionEvaluation(*value, State::CONTINUE_EVALUATED);
      }, std::move(start_reactor), std::move(start_updates),
      std::move(stop_reactor), std::move(stop_updates), std::move(step_reactor),
      std::move(step_updates));
  }

  /**
   * Makes a reactor that counts from a starting value to an end value
   * (inclusive).
   * @param start The first value to evaluate to.
   * @param stop The value to stop evaluating at (exclusive).
   */
  template<typename S, typename E>
  auto range(S&& start, E&& stop) {
    return range(std::forward<S>(start), std::forward<E>(stop), constant(1));
  }
}

#endif

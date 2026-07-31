#ifndef ASPEN_UNCONSECUTIVE_HPP
#define ASPEN_UNCONSECUTIVE_HPP
#include <concepts>
#include <optional>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that only evaluates when unconsecutive values are
   * produced.
   * @param series The series producing the values.
   * @return A reactor evaluating to the <i>series</i> whenever it produces a
   *         value differing from the one before it.
   */
  template<typename Series> requires IsReactor<to_reactor_t<Series>> &&
    std::equality_comparable<reactor_result_t<Series>>
  auto unconsecutive(Series&& series) {
    using Type = reactor_result_t<Series>;
    using Source = to_reactor_t<Series>;
    return lift([previous = std::optional<Type>()] (
        const auto& value) mutable noexcept -> FunctionEvaluation<Type> {
      if constexpr(!is_noexcept_reactor_v<Source>) {
        if(value.has_exception()) {
          return Maybe<Type>(value.get_exception());
        }
      }
      auto& current = static_cast<const Type&>(value);
      if(previous == current) {
        return State::NONE;
      }
      previous = current;
      return *previous;
    }, std::forward<Series>(series));
  }
}

#endif

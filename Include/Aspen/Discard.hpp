#ifndef ASPEN_DISCARD_HPP
#define ASPEN_DISCARD_HPP
#include <optional>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that discards values produced by its child whenever
   * a condition evaluates to true.
   * @param toggle The indicator used to determine whether values should be
   *               discarded.
   * @param series The series producing the values.
   * @return A reactor evaluating to the <i>series</i> whenever the
   *         <i>toggle</i> is false.
   */
  template<typename Toggle, typename Series> requires
    IsReactorOf<to_reactor_t<Toggle>, bool> && IsReactor<to_reactor_t<Series>>
  auto discard(Toggle&& toggle, Series&& series) {
    using Type = reactor_result_t<Series>;
    return lift([] (const auto& toggle, const auto& series) noexcept(
        is_noexcept_reactor_v<to_reactor_t<Toggle>> &&
        is_noexcept_reactor_v<to_reactor_t<Series>>) -> std::optional<Type> {
      if(toggle) {
        return std::nullopt;
      }
      return series;
    }, std::forward<Toggle>(toggle), std::forward<Series>(series));
  }
}

#endif

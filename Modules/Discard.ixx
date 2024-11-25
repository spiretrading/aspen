export module Aspen:Discard;

import std;
import :Lift;

export namespace Aspen {
namespace Details {
  template<typename Type>
  struct DiscardCore {
    template<typename T>
    std::optional<Type> operator ()(bool toggle, T&& series) const noexcept {
      if(toggle) {
        return std::nullopt;
      }
      return std::forward<T>(series);
    }
  };
}

  /**
   * Implements a reactor that discards values produced by its child whenever
   * a condition evaluates to true.
   * @param toggle The indicator used to determine whether values should be
   *               discarded.
   * @param series The series producing the values.
   */
  template<typename Toggle, typename Series>
  auto discard(Toggle&& toggle, Series&& series) {
    using Type = reactor_result_t<Series>;
    return Lift(Details::DiscardCore<Type>(), std::forward<Toggle>(toggle),
      std::forward<Series>(series));
  }
}

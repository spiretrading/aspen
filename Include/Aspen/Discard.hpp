#ifndef ASPEN_DISCARD_HPP
#define ASPEN_DISCARD_HPP
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Switch.hpp"

namespace Aspen {

  /**
   * Implements a reactor that discards values produced by its child whenever
   * a condition evaluates to true.
   * @param toggle The indicator used to determine whether values should be
   *               discarded.
   * @param series The series producing the values.
   */
  template<typename Toggle, typename Series>
  auto discard(Toggle&& toggle, Series&& series) {
    auto series_reactor = make_ptr(std::forward<Series>(series));
    return Switch(Lift([] (auto&& t, auto&& s) noexcept -> decltype(auto) {
      return std::forward<decltype(t)>(t);
    }, std::forward<Toggle>(toggle), std::move(series_reactor)),
      &*series_reactor);
  }
}

#endif

#ifndef ASPEN_FIRST_HPP
#define ASPEN_FIRST_HPP
#include <utility>
#include "Aspen/Lift.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to the first value it receives from its
   * source.
   * @param source The source that will provide the value to evaluate to.
   */
  template<typename Reactor>
  auto first(Reactor&& source) {
    return Lift(
      [] (auto&& value) noexcept {
        return std::forward<decltype(value)>(value);
      }, std::forward<Reactor>(source));
  }
}

#endif

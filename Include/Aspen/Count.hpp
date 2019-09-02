#ifndef ASPEN_COUNT_HPP
#define ASPEN_COUNT_HPP
#include <utility>
#include "Aspen/Lift.hpp"

namespace Aspen {

  /** Counts the number of evaluations produced by a reactor. */
  template<typename T>
  auto count(T&& series) {
    return lift(
      [counter = 0] (auto&& value) mutable noexcept {
        ++counter;
        return counter;
      }, std::forward<T>(series));
  }
}

#endif

export module Aspen:Count;

import std;
import :Lift;

export namespace Aspen {

  /** Counts the number of evaluations produced by a reactor. */
  template<typename T>
  auto count(T&& series) {
    return lift([counter = 0] (auto&& value) mutable noexcept {
      ++counter;
      return counter;
    }, std::forward<T>(series));
  }
}

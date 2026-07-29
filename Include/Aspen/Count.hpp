#ifndef ASPEN_COUNT_HPP
#define ASPEN_COUNT_HPP
#include <cstdint>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Counts the number of evaluations produced by a reactor.
   * @param series The reactor whose evaluations are counted.
   * @return A reactor evaluating to the number of evaluations produced.
   */
  template<typename Series> requires IsReactor<to_reactor_t<Series>>
  auto count(Series&& series) {
    return lift(
      [counter = std::uint64_t(0)] (const auto& value) mutable noexcept {
        ++counter;
        if constexpr(is_noexcept_reactor_v<to_reactor_t<Series>>) {
          return counter;
        } else {
          if(value.has_exception()) {
            return Maybe<std::uint64_t>(value.get_exception());
          }
          return Maybe(counter);
        }
      }, std::forward<Series>(series));
  }
}

#endif

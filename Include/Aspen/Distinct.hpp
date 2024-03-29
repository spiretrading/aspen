#ifndef ASPEN_DISTINCT_HPP
#define ASPEN_DISTINCT_HPP
#include <unordered_set>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that only evaluates distinct values, ignoring values
   * that have been previously evaluated.
   * @param source The source to filter out duplicate values from.
   */
  template<typename Reactor>
  auto distinct(Reactor&& source) {
    using Type = reactor_result_t<Reactor>;
    return Lift([production = std::unordered_set<Type>()] (
        auto&& value) mutable noexcept -> FunctionEvaluation<Type> {
      auto result = production.insert(value);
      if(result.second) {
        return std::forward<decltype(value)>(value);
      }
      return State::NONE;
    }, std::forward<Reactor>(source));
  }
}

#endif

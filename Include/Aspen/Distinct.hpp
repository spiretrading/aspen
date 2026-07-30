#ifndef ASPEN_DISTINCT_HPP
#define ASPEN_DISTINCT_HPP
#include <concepts>
#include <functional>
#include <unordered_set>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that only evaluates distinct values, ignoring values
   * that have been previously evaluated.
   * @param source The source to filter out duplicate values from.
   * @return A reactor evaluating to the distinct values of the <i>source</i>.
   */
  template<typename Source> requires IsReactor<to_reactor_t<Source>> &&
    std::equality_comparable<reactor_result_t<Source>> &&
    requires(const reactor_result_t<Source>& value) {
      std::hash<reactor_result_t<Source>>()(value);
    }
  auto distinct(Source&& source) {
    using Type = reactor_result_t<Source>;
    return lift([production = std::unordered_set<Type>()] (
        const auto& value) mutable noexcept -> FunctionEvaluation<Type> {
      if constexpr(!is_noexcept_reactor_v<to_reactor_t<Source>>) {
        if(value.has_exception()) {
          return value;
        }
      }
      if(production.insert(value).second) {
        return value;
      }
      return State::NONE;
    }, std::forward<Source>(source));
  }
}

#endif

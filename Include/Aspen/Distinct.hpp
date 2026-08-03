#ifndef ASPEN_DISTINCT_HPP
#define ASPEN_DISTINCT_HPP
#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Hashes the values evaluated by a distinct reactor.
   * @param <T> The type of value to hash.
   */
  template<typename T>
  struct DistinctHash {
    std::size_t operator ()(const T& value) const
      noexcept(noexcept(std::hash<T>()(value))) requires
        std::invocable<std::hash<T>, const T&>;
  };

  /**
   * Tests whether two values evaluated by a distinct reactor are duplicates.
   * @param <T> The type of value to compare.
   */
  template<typename T>
  struct DistinctEquality {
    bool operator ()(const T& left, const T& right) const
      noexcept(noexcept(left == right)) requires std::equality_comparable<T>;
  };

  /**
   * Implements a reactor that only evaluates distinct values, ignoring values
   * that have been previously evaluated.
   * @param source The source to filter out duplicate values from.
   * @return A reactor evaluating to the distinct values of the <i>source</i>.
   */
  template<typename Source> requires IsReactor<to_reactor_t<Source>> &&
    requires(const reactor_result_t<Source>& value) {
      DistinctHash<reactor_result_t<Source>>()(value);
      DistinctEquality<reactor_result_t<Source>>()(value, value);
    }
  auto distinct(Source&& source) {
    using Type = reactor_result_t<Source>;
    using Production =
      std::unordered_set<Type, DistinctHash<Type>, DistinctEquality<Type>>;
    return lift([production = Production()] (const auto& value) mutable
        noexcept(
          std::is_nothrow_invocable_v<DistinctHash<Type>, const Type&> &&
          std::is_nothrow_invocable_v<
            DistinctEquality<Type>, const Type&, const Type&>) ->
            FunctionEvaluation<std::remove_cvref_t<decltype(value)>> {
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

  template<typename T>
  std::size_t DistinctHash<T>::operator ()(const T& value) const
      noexcept(noexcept(std::hash<T>()(value))) requires
      std::invocable<std::hash<T>, const T&> {
    return std::hash<T>()(value);
  }

  template<typename T>
  bool DistinctEquality<T>::operator ()(const T& left, const T& right) const
      noexcept(noexcept(left == right)) requires std::equality_comparable<T> {
    return left == right;
  }
}

#endif

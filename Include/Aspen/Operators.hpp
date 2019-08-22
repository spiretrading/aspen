#ifndef ASPEN_OPERATORS_HPP
#define ASPEN_OPERATORS_HPP
#include <type_traits>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Adds two reactors together.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that adds its two operands together.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator +(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left + right)) {
      return left + right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Subtracts two reactors from each other.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that subtracts its two operands from each other.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator -(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left - right)) {
      return left - right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Multiplies two reactors with each other.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that multiplies its two operands together.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator *(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left * right)) {
      return left * right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Divides two reactors with by each other.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that divides its two operands by each other.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator /(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left / right)) {
      return left / right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the modulus of two reactors.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that takes the modulus of its operands.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator %(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left % right)) {
      return left % right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the binary XOR of two reactors.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that takes the binary XOR of its operands.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator ^(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left ^ right)) {
      return left ^ right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the binary AND of two reactors.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that takes the binary AND of its operands.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator &(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left & right)) {
      return left & right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the binary OR of two reactors.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that takes the binary OR of its operands.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator |(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left | right)) {
      return left | right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the binary negation of a reactor.
   * @param series The series to negate.
   * @return A reactor that takes the binary negation of its operand.
   */
  template<typename T, typename = std::enable_if_t<is_reactor_or_pointer_v<T>>>
  auto operator ~(T&& series) {
    using Type = reactor_result_t<T>;
    return lift([] (const Type& value) noexcept(noexcept(~value)) {
      return ~value;
    }, std::forward<T>(series));
  }

  /**
   * Takes the logical negation of a reactor.
   * @param series The series to negate.
   * @return A reactor that takes the logical negation of its operand.
   */
  template<typename T, typename = std::enable_if_t<is_reactor_or_pointer_v<T>>>
  auto operator !(T&& series) {
    using Type = reactor_result_t<T>;
    return lift([] (const Type& value) noexcept(noexcept(!value)) {
      return !value;
    }, std::forward<T>(series));
  }

  /**
   * Applies the left-shift operator to its operands.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that applies the left-shift operator to its operands.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator <<(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left << right)) {
      return left << right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Applies the right-shift operator to its operands.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that applies the right-shift operator to its operands.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator >>(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left >> right)) {
      return left >> right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Tests if one reactor is less than another.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that tests if its left operand is less than its right
   *         operand.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator <(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left < right)) {
      return left < right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Tests if one reactor is less than or equal to another.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that tests if its left operand is less than or equal to
   *         its right operand.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator <=(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left <= right)) {
      return left <= right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Tests if its two reactors are equal.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that tests if its operands are equal.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_v<L> && is_reactor_or_pointer_v<R> ||
    is_reactor_or_pointer_v<L> && is_reactor_v<R>>>
  auto operator ==(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left == right)) {
      return left == right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Tests if its two reactors are not equal.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that tests if its operands are not equal.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_v<L> && is_reactor_or_pointer_v<R> ||
    is_reactor_or_pointer_v<L> && is_reactor_v<R>>>
  auto operator !=(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left != right)) {
      return left != right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Tests if one reactor is greater than or equal to another.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that tests if its left operand is greater than or equal
   *         to its right operand.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator >=(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left >= right)) {
      return left >= right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Tests if one reactor is greater than another.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that tests if its left operand is greater than its right
   *         operand.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator >(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left > right)) {
      return left > right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Negates its operand.
   * @param series The series to negate.
   * @return A reactor that negates its operand.
   */
  template<typename T, typename = std::enable_if_t<is_reactor_or_pointer_v<T>>>
  auto operator -(T&& series) {
    using Type = reactor_result_t<T>;
    return lift([] (const Type& value) noexcept(noexcept(-value)) {
      return -value;
    }, std::forward<T>(series));
  }

  /**
   * Plusses its operand.
   * @param series The series to plus.
   * @return A reactor that plusses its operand.
   */
  template<typename T, typename = std::enable_if_t<is_reactor_or_pointer_v<T>>>
  auto operator +(T&& series) {
    using Type = reactor_result_t<T>;
    return lift([] (const Type& value) noexcept(noexcept(+value)) {
      return +value;
    }, std::forward<T>(series));
  }

  /**
   * Takes the logical AND of its operands.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that takes the logical AND of its operands.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator &&(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left && right)) {
      return left && right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the logical OR of its operands.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that takes the logical AND of its operands.
   */
  template<typename L, typename R, typename =
    std::enable_if_t<is_reactor_or_pointer_v<L> && is_reactor_or_pointer_v<R>>>
  auto operator ||(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right)
        noexcept(noexcept(left || right)) {
      return left || right;
    }, std::forward<L>(left), std::forward<R>(right));
  }
}

#endif

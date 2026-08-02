#ifndef ASPEN_OPERATORS_HPP
#define ASPEN_OPERATORS_HPP
#include <type_traits>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Adds two reactors together.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that adds its two operands together.
   */
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator +(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left + right)) {
      return left + right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Subtracts two reactors from each other.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that subtracts its two operands from each other.
   */
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator -(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left - right)) {
      return left - right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Multiplies two reactors with each other.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that multiplies its two operands together.
   */
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator *(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left * right)) {
      return left * right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Divides one reactor by another.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that divides its left operand by its right operand.
   */
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator /(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left / right)) {
      return left / right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the modulus of two reactors.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that takes the modulus of its operands.
   */
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator %(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left % right)) {
      return left % right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the bitwise XOR of two reactors.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that takes the bitwise XOR of its operands.
   */
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator ^(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left ^ right)) {
      return left ^ right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the bitwise AND of two reactors.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that takes the bitwise AND of its operands.
   */
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator &(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left & right)) {
      return left & right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the bitwise OR of two reactors.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that takes the bitwise OR of its operands.
   */
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator |(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left | right)) {
      return left | right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the bitwise negation of a reactor.
   * @param series The series to negate.
   * @return A reactor that takes the bitwise negation of its operand.
   */
  template<typename T> requires IsReactor<std::remove_cvref_t<T>>
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
  template<typename T> requires IsReactor<std::remove_cvref_t<T>>
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
  template<typename L, typename R> requires IsReactor<std::remove_cvref_t<L>>
  auto operator <<(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left << right)) {
      return left << right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Applies the right-shift operator to its operands.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that applies the right-shift operator to its operands.
   */
  template<typename L, typename R> requires IsReactor<std::remove_cvref_t<L>>
  auto operator >>(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left >> right)) {
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
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator <(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left < right)) {
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
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator <=(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left <= right)) {
      return left <= right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Tests if its two reactors are equal.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that tests if its operands are equal.
   */
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator ==(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left == right)) {
      return left == right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Tests if its two reactors are not equal.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that tests if its operands are not equal.
   */
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator !=(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left != right)) {
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
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator >=(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left >= right)) {
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
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator >(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left > right)) {
      return left > right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Negates its operand.
   * @param series The series to negate.
   * @return A reactor that negates its operand.
   */
  template<typename T> requires IsReactor<std::remove_cvref_t<T>>
  auto operator -(T&& series) {
    using Type = reactor_result_t<T>;
    return lift([] (const Type& value) noexcept(noexcept(-value)) {
      return -value;
    }, std::forward<T>(series));
  }

  /**
   * Applies unary plus to its operand.
   * @param series The series to apply unary plus to.
   * @return A reactor that applies unary plus to its operand.
   */
  template<typename T> requires IsReactor<std::remove_cvref_t<T>>
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
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator &&(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left && right)) {
      return left && right;
    }, std::forward<L>(left), std::forward<R>(right));
  }

  /**
   * Takes the logical OR of its operands.
   * @param left The left hand side of the operation.
   * @param right The right hand side of the operation.
   * @return A reactor that takes the logical OR of its operands.
   */
  template<typename L, typename R> requires
    IsReactor<std::remove_cvref_t<L>> || IsReactor<std::remove_cvref_t<R>>
  auto operator ||(L&& left, R&& right) {
    using Left = reactor_result_t<L>;
    using Right = reactor_result_t<R>;
    return lift([] (const Left& left, const Right& right) noexcept(
        noexcept(left || right)) {
      return left || right;
    }, std::forward<L>(left), std::forward<R>(right));
  }
}

#endif

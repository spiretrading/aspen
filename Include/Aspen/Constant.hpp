#ifndef ASPEN_CONSTANT_HPP
#define ASPEN_CONSTANT_HPP
#include <cstdint>
#include <type_traits>
#include <utility>
#include "Aspen/State.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to a constant.
   * @param <T> The type of the constant to evaluate to.
   */
  template<typename T>
  class Constant {
    public:

      /** The type of the constant to evaluate to. */
      using Type = T;

      /**
       * Constructs a Constant.
       * @param value The constant to evaluate to.
       */
      constexpr explicit Constant(T value) noexcept(
        std::is_nothrow_move_constructible_v<T>);

      constexpr State commit(std::uint64_t sequence) noexcept;
      constexpr const Type& eval() const noexcept;

    private:
      [[no_unique_address]]
      Type m_value;
  };

  /**
   * Wraps a constant value in a reactor.
   * @param value The value to wrap.
   * @return A reactor evaluating to the <i>value</i>.
   */
  constexpr auto constant(auto&& value) {
    return Constant(std::forward<decltype(value)>(value));
  }

  template<typename T>
  constexpr Constant<T>::Constant(T value) noexcept(
    std::is_nothrow_move_constructible_v<T>)
    : m_value(std::move(value)) {}

  template<typename T>
  constexpr State Constant<T>::commit(std::uint64_t sequence) noexcept {
    return State::COMPLETE_EVALUATED;
  }

  template<typename T>
  constexpr const T& Constant<T>::eval() const noexcept {
    return m_value;
  }
}

#endif

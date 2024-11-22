export module Aspen:Constant;

import <utility>;
import :State;

export namespace Aspen {

  /**
   * Implements a reactor that evaluates to a constant.
   * @param <T> The type of the constant to evaluate to.
   */
  template<typename T>
  class Constant {
    public:
      using Type = T;

      /**
       * Constructs a Constant.
       * @param value The constant to evaluate to.
       */
      constexpr explicit Constant(T value);

      constexpr State commit(int sequence) noexcept;

      constexpr const Type& eval() const noexcept;

    private:
      Type m_value;
  };

  /**
   * Wraps a constant value in a reactor.
   * @param value The value to wrap.
   */
  template<typename T>
  constexpr auto constant(T&& value) {
    return Constant(std::forward<T>(value));
  }

  template<typename T>
  constexpr Constant<T>::Constant(T value)
    : m_value(std::move(value)) {}

  template<typename T>
  constexpr State Constant<T>::commit(int sequence) noexcept {
    return State::COMPLETE_EVALUATED;
  }

  template<typename T>
  constexpr const typename Constant<T>::Type& Constant<T>::eval()
      const noexcept {
    return m_value;
  }
}

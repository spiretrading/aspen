#ifndef ASPEN_CONSTANT_HPP
#define ASPEN_CONSTANT_HPP
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
      using Type = T;

      /**
       * Constructs a Constant.
       * @param value The constant to evaluate to.
       */
      constexpr Constant(T value);

      constexpr State commit(int sequence);

      constexpr const Type& eval() const;

    private:
      Type m_value;
  };

  template<typename T>
  constexpr Constant<T>::Constant(T value)
      : m_value(std::move(value)) {}

  template<typename T>
  constexpr State Constant<T>::commit(int sequence) {
    return State::COMPLETE_EVALUATED;
  }

  template<typename T>
  constexpr const typename Constant<T>::Type& Constant<T>::eval() const {
    return m_value;
  }
}

#endif

#ifndef ASPEN_NONE_HPP
#define ASPEN_NONE_HPP
#include <exception>
#include "Aspen/State.hpp"

namespace Aspen {

  /**
   * Implements a reactor that never produces an evaluation.
   * @param <T> The type of the value to evaluate to.
   */
  template<typename T>
  class None {
    public:
      using Type = T;

      /**
       * Constructs a None reactor.
       */
      constexpr None() = default;

      constexpr State commit(int sequence);

      constexpr const Type& eval() const;
  };

  template<typename T>
  constexpr State None<T>::commit(int sequence) {
    return State::COMPLETE_EMPTY;
  }

  template<typename T>
  constexpr const typename None<T>::Type& None<T>::eval() const {
    throw std::runtime_error("No evaluation.");
  }
}

#endif

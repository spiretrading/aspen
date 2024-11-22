export module Aspen:None;

import <exception>;
import :State;
import :Traits;

export namespace Aspen {

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

      constexpr State commit(int sequence) noexcept;

      constexpr eval_result_t<Type> eval() const;
  };

  /**
   * Returns a reactor that never produces an evaluation.
   */
  template<typename T>
  constexpr auto none() {
    return None<T>();
  }

  template<typename T>
  constexpr State None<T>::commit(int sequence) noexcept {
    return State::COMPLETE;
  }

  template<typename T>
  constexpr eval_result_t<typename None<T>::Type> None<T>::eval() const {
    throw std::runtime_error("No evaluation.");
  }
}

export module Aspen:Throw;

import <exception>;
import <type_traits>;
import <utility>;
import :State;
import :Traits;

export namespace Aspen {

  /**
   * Implements a reactor that unconditionally throws.
   * @param <T> The type of reactor to evaluate to.
   */
  template<typename T>
  class Throw {
    public:
      using Type = T;

      /**
       * Constructs a Throw.
       * @param exception The exception to throw.
       */
      explicit Throw(std::exception_ptr exception);

      /**
       * Constructs a Throw.
       * @param exception The exception to throw.
       */
      template<typename E>
      Throw(E exception);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const;

    private:
      std::exception_ptr m_exception;
  };

  /**
   * Returns a reactor that always throws an exception.
   * @param exception The exception to throw.
   */
  template<typename T>
  auto throws(std::exception_ptr exception) {
    return Throw<T>(std::move(exception));
  }

  /**
   * Returns a reactor that always throws an exception.
   * @param exception The exception to throw.
   */
  template<typename T, typename E>
  auto throws(E exception) {
    return Throw<T>(std::move(exception));
  }

  template<typename T>
  Throw<T>::Throw(std::exception_ptr exception)
    : m_exception(std::move(exception)) {}

  template<typename T>
  template<typename E>
  Throw<T>::Throw(E exception)
    : Throw(std::make_exception_ptr(std::move(exception))) {}

  template<typename T>
  State Throw<T>::commit(int sequence) noexcept {
    return State::COMPLETE_EVALUATED;
  }

  template<typename T>
  eval_result_t<typename Throw<T>::Type> Throw<T>::eval() const {
    std::rethrow_exception(m_exception);
    if constexpr(!std::is_same_v<Type, void>) {
      return *static_cast<const Type*>(nullptr);
    }
  }
}

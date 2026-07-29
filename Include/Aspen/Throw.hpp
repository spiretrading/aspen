#ifndef ASPEN_THROW_HPP
#define ASPEN_THROW_HPP
#include <cstdint>
#include <exception>
#include <utility>
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that unconditionally throws.
   * @param <T> The type of reactor to evaluate to.
   */
  template<typename T>
  class Throw {
    public:

      /** The type to evaluate to. */
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
      explicit Throw(E exception);

      State commit(std::uint64_t sequence) noexcept;
      eval_result_t<Type> eval() const;

    private:
      std::exception_ptr m_exception;
  };

  /**
   * Returns a reactor that always throws an exception.
   * @param <T> The type of reactor to evaluate to.
   * @param exception The exception to throw.
   * @return A reactor that throws the <i>exception</i>.
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
  State Throw<T>::commit(std::uint64_t sequence) noexcept {
    return State::COMPLETE_EVALUATED;
  }

  template<typename T>
  eval_result_t<typename Throw<T>::Type> Throw<T>::eval() const {
    std::rethrow_exception(m_exception);
  }
}

#endif

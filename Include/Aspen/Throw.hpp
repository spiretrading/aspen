#ifndef ASPEN_THROW_HPP
#define ASPEN_THROW_HPP
#include <concepts>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <utility>
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that unconditionally throws.
   * @param <T> The type of the value to evaluate to.
   */
  template<typename T>
  class Throw {
    public:

      /** The type to evaluate to. */
      using Type = T;

      /**
       * Constructs a Throw.
       * @param exception The exception to throw, where a null exception
       *        throws a std::runtime_error instead.
       */
      explicit Throw(std::exception_ptr exception) noexcept;

      /**
       * Constructs a Throw.
       * @param exception The exception to throw.
       */
      template<std::derived_from<std::exception> E>
      explicit Throw(E exception) noexcept;

      State commit(std::uint64_t sequence) noexcept;
      eval_result_t<Type> eval() const;

    private:
      std::exception_ptr m_exception;
  };

  /**
   * Returns a reactor that always throws an exception.
   * @param <T> The type of the value to evaluate to.
   * @param exception The exception to throw.
   * @return A reactor that throws the <i>exception</i>.
   */
  template<typename T, std::derived_from<std::exception> E>
  auto throws(E exception) {
    return Throw<T>(std::move(exception));
  }

  template<typename T>
  Throw<T>::Throw(std::exception_ptr exception) noexcept
    : m_exception(std::move(exception)) {}

  template<typename T>
  template<std::derived_from<std::exception> E>
  Throw<T>::Throw(E exception) noexcept
    : Throw(std::make_exception_ptr(std::move(exception))) {}

  template<typename T>
  State Throw<T>::commit(std::uint64_t sequence) noexcept {
    return State::COMPLETE_EVALUATED;
  }

  template<typename T>
  eval_result_t<typename Throw<T>::Type> Throw<T>::eval() const {
    if(!m_exception) {
      throw std::runtime_error("Uninitialized.");
    }
    std::rethrow_exception(m_exception);
  }
}

#endif

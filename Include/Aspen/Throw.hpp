#ifndef ASPEN_THROW_HPP
#define ASPEN_THROW_HPP
#include <exception>
#include <utility>
#include "Aspen/State.hpp"

namespace Aspen {

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
      Throw(std::exception_ptr exception);

      /**
       * Constructs a Throw.
       * @param exception The exception to throw.
       */
      template<typename E>
      Throw(E exception);

      State commit(int sequence);

      const Type& eval() const;

    private:
      std::exception_ptr m_exception;
  };

  template<typename T>
  Throw<T>::Throw(std::exception_ptr exception)
    : m_exception(std::move(exception)) {}

  template<typename T>
  template<typename E>
  Throw<T>::Throw(E exception)
    : Throw(std::make_exception_ptr(std::move(exception))) {}

  template<typename T>
  State Throw<T>::commit(int sequence) {
    return State::COMPLETE_EVALUATED;
  }

  template<typename T>
  const typename Throw<T>::Type& Throw<T>::eval() const {
    std::rethrow_exception(m_exception);
    return *static_cast<const T*>(nullptr);
  }
}

#endif

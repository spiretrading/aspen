#ifndef ASPEN_PYTHON_PYTHON_BOX_HPP
#define ASPEN_PYTHON_PYTHON_BOX_HPP
#include <cstdint>
#include <exception>
#include <pybind11/pybind11.h>
#include <utility>
#include "Aspen/Python/Exception.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Wraps a Python object implementing a reactor into a box.
   * @param <T> The type of value to evaluate to.
   */
  template<typename T>
  class PythonBox {
    public:
      using Type = T;

      /**
       * Constructs a PythonBox from a Python object.
       * @param reactor The Python object implementing the reactor to box.
       */
      explicit PythonBox(pybind11::object reactor);

      State commit(std::uint64_t sequence) noexcept;

      Type eval() const;

    private:
      pybind11::object m_reactor;
      std::exception_ptr m_exception;
  };

  template<typename T>
  PythonBox<T>::PythonBox(pybind11::object reactor)
    : m_reactor(std::move(reactor)) {}

  template<typename T>
  State PythonBox<T>::commit(std::uint64_t sequence) noexcept {
    try {
      return m_reactor.attr("commit")(sequence).template cast<State>();
    } catch(const pybind11::error_already_set& error) {
      m_exception = std::make_exception_ptr(PythonException(error));
    } catch(...) {
      m_exception = std::current_exception();
    }
    return State::COMPLETE_EVALUATED;
  }

  template<typename T>
  typename PythonBox<T>::Type PythonBox<T>::eval() const {
    if(m_exception) {
      std::rethrow_exception(m_exception);
    }
    try {
      return m_reactor.attr("eval")().template cast<T>();
    } catch(const pybind11::error_already_set& error) {
      throw PythonException(error);
    }
  }
}

#endif

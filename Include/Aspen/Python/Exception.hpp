#ifndef ASPEN_PYTHON_EXCEPTION_HPP
#define ASPEN_PYTHON_EXCEPTION_HPP
#include <exception>
#include <stdexcept>
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Stores a Python exception so that it may be raised more than once. */
  class PythonException : public std::runtime_error {
    public:

      /**
       * Constructs a PythonException from an error raised by Python.
       * @param error The error to store.
       */
      explicit PythonException(const pybind11::error_already_set& error);

      /** Makes this exception Python's active error. */
      void restore() const;

    private:
      pybind11::object m_type;
      pybind11::object m_value;
      pybind11::object m_trace;
  };

  /** Registers the translation of a PythonException back into Python. */
  void register_python_exception();

  inline PythonException::PythonException(
    const pybind11::error_already_set& error)
    : std::runtime_error(error.what()),
      m_type(error.type()),
      m_value(error.value()),
      m_trace(error.trace()) {}

  inline void PythonException::restore() const {
    PyErr_Restore(m_type.inc_ref().ptr(), m_value.inc_ref().ptr(),
      m_trace.inc_ref().ptr());
  }

  inline void register_python_exception() {
    pybind11::register_exception_translator([] (std::exception_ptr error) {
      try {
        if(error) {
          std::rethrow_exception(error);
        }
      } catch(const PythonException& exception) {
        exception.restore();
      }
    });
  }
}

#endif

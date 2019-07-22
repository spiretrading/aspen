#ifndef ASPEN_PYTHON_BOX_HPP
#define ASPEN_PYTHON_BOX_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Aspen/Box.hpp"

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
      PythonBox(pybind11::object reactor);

      State commit(int sequence);

      Type eval() const;

    private:
      pybind11::object m_reactor;
  };

  /** Exports a Box evaluating to a Python object. */
  void export_box(pybind11::module& module);

  /**
   * Exports the generic Box reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_box(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Box";
    pybind11::class_<Box<T>>(module, name.c_str())
      .def(pybind11::init(
        [] (pybind11::object reactor) {
          return Box(PythonBox<T>(std::move(reactor)));
        }))
      .def("commit", &Box<T>::commit)
      .def("eval", &Box<T>::eval);
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<Box<T>, Box<pybind11::object>>();
      pybind11::implicitly_convertible<Box<pybind11::object>, Box<T>>();
    }
  }

  template<typename T>
  PythonBox<T>::PythonBox(pybind11::object reactor)
    : m_reactor(std::move(reactor)) {}

  template<typename T>
  State PythonBox<T>::commit(int sequence) {
    return m_reactor.attr("commit")(sequence).cast<State>();
  }

  template<typename T>
  typename PythonBox<T>::Type PythonBox<T>::eval() const {
    return m_reactor.attr("eval")().cast<T>();
  }
}

#endif

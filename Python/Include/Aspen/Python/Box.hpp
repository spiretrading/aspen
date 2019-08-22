#ifndef ASPEN_PYTHON_BOX_HPP
#define ASPEN_PYTHON_BOX_HPP
#include <string>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Aspen/Box.hpp"
#include "Aspen/Python/Reactor.hpp"

namespace Aspen {

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
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<Box<T>>(module, name)
      .def(pybind11::init(
        [] (pybind11::object reactor) {
          return to_python_reactor<T>(std::move(reactor));
        }));
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<Box<T>, Box<pybind11::object>>();
      pybind11::implicitly_convertible<Box<pybind11::object>, Box<T>>();
    }
    pybind11::implicitly_convertible<pybind11::object, Box<T>>();
  }
}

#endif

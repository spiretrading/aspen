#ifndef ASPEN_PYTHON_CONSTANT_HPP
#define ASPEN_PYTHON_CONSTANT_HPP
#include <string>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Aspen/Constant.hpp"
#include "Aspen/Python/Reactor.hpp"

namespace Aspen {

  /** Exports a Constant evaluating to a Python object. */
  void export_constant(pybind11::module& module);

  /**
   * Exports the generic Constant class.
   * @param module The module to export the class to.
   * @param prefix The name of the type the Constant generates.
   */
  template<typename T>
  void export_constant(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + std::string("Constant");
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<Constant<T>>(module, name)
      .def(pybind11::init<T>());
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<Constant<T>,
        Constant<pybind11::object>>();
    }
  }
}

#endif

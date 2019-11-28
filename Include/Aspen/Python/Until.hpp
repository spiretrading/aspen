#ifndef ASPEN_PYTHON_UNTIL_HPP
#define ASPEN_PYTHON_UNTIL_HPP
#include <string>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Aspen/Until.hpp"
#include "Aspen/Python/Reactor.hpp"

namespace Aspen {

  /** Exports an Until evaluating to a Python object. */
  void export_until(pybind11::module& module);

  /**
   * Exports the generic Until reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename C, typename T>
  void export_until(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Until";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<Until<C, T>>(module, name)
      .def(pybind11::init<C, T>());
    if constexpr(!std::is_same_v<C, SharedBox<bool>> ||
        !std::is_same_v<T, SharedBox<pybind11::object>>) {
      pybind11::implicitly_convertible<Until<C, T>, Until<SharedBox<bool>,
        SharedBox<pybind11::object>>>();
    }
  }
}

#endif

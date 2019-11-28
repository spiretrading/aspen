#ifndef ASPEN_PYTHON_SWITCH_HPP
#define ASPEN_PYTHON_SWITCH_HPP
#include <string>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Aspen/Switch.hpp"
#include "Aspen/Python/Reactor.hpp"

namespace Aspen {

  /** Exports a Switch evaluating to a Python object. */
  void export_switch(pybind11::module& module);

  /**
   * Exports the generic Switch reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T, typename S>
  void export_switch(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Switch";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<Switch<T, S>>(module, name)
      .def(pybind11::init<T, S>());
    if constexpr(!std::is_same_v<T, SharedBox<bool>> ||
        !std::is_same_v<S, SharedBox<pybind11::object>>) {
      pybind11::implicitly_convertible<Switch<T, S>,
        Switch<SharedBox<bool>, SharedBox<pybind11::object>>>();
    }
  }
}

#endif

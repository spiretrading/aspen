#ifndef ASPEN_PYTHON_SWITCH_HPP
#define ASPEN_PYTHON_SWITCH_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Aspen/Switch.hpp"
#include "Aspen/Python/Box.hpp"

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
    pybind11::class_<Switch<T, S>>(module, name.c_str())
      .def(pybind11::init<T, S>())
      .def("commit", &Switch<T, S>::commit)
      .def("eval", &Switch<T, S>::eval);
    implicitly_convertible_to_box<Switch<T, S>>();
    if constexpr(!std::is_same_v<T, Box<bool>> ||
        !std::is_same_v<S, Box<pybind11::object>>) {
      pybind11::implicitly_convertible<Switch<T, S>,
        Switch<Box<bool>, Box<pybind11::object>>>();
    }
  }
}

#endif

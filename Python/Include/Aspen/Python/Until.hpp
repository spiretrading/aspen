#ifndef ASPEN_PYTHON_UNTIL_HPP
#define ASPEN_PYTHON_UNTIL_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Aspen/Python/Box.hpp"
#include "Aspen/Until.hpp"

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
    pybind11::class_<Until<C, T>>(module, name.c_str())
      .def(pybind11::init<C, T>())
      .def("commit", &Until<C, T>::commit)
      .def("eval", &Until<C, T>::eval);
    implicitly_convertible_to_box<Until<C, T>>();
    if constexpr(!std::is_same_v<C, Box<bool>> ||
        !std::is_same_v<T, Box<pybind11::object>>) {
      pybind11::implicitly_convertible<Until<C, T>, Until<Box<bool>,
        Box<pybind11::object>>>();
    }
  }
}

#endif

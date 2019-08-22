#ifndef ASPEN_PYTHON_NONE_HPP
#define ASPEN_PYTHON_NONE_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Aspen/None.hpp"
#include "Aspen/Python/Box.hpp"

namespace Aspen {

  /** Exports a None evaluating to a Python object. */
  void export_none(pybind11::module& module);

  /**
   * Exports the generic None class.
   * @param module The module to export the class to.
   * @param prefix The name of the type the None generates.
   */
  template<typename T>
  void export_none(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + std::string("None");
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<None<T>>(module, name.c_str())
      .def(pybind11::init<>())
      .def("commit", &None<T>::commit)
      .def("eval", &None<T>::eval);
    implicitly_convertible_to_box<None<T>>();
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<None<T>, None<pybind11::object>>();
    }
  }
}

#endif

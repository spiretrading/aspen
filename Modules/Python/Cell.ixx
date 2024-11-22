export module Aspen:Cell;

import <string>;
import <type_traits>;
import <pybind11/pybind11.h>;
import :Cell;
#include "Aspen/Python/Reactor.hpp"

export namespace Aspen {

  /** Exports a Cell evaluating to a Python object. */
  void export_cell(pybind11::module& module);

  /**
   * Exports the generic Cell class.
   * @param module The module to export the class to.
   * @param prefix The name of the type the Cell generates.
   */
  template<typename T>
  void export_cell(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + std::string("Cell");
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<Cell<T>>(module, name)
      .def(pybind11::init<>())
      .def(pybind11::init<T>())
      .def("set", &Cell<T>::set)
      .def("set_complete",
        static_cast<void (Cell<T>::*)()>(&Cell<T>::set_complete))
      .def("set_complete",
        static_cast<void (Cell<T>::*)(T)>(&Cell<T>::set_complete));
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<Cell<T>, Cell<pybind11::object>>();
    }
  }
}

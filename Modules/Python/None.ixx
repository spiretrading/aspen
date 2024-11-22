export module Aspen:None;

import <string>;
import <type_traits>;
import <pybind11/pybind11.h>;
import :None;
#include "Aspen/Python/Reactor.hpp"

export namespace Aspen {

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
    export_reactor<None<T>>(module, name)
      .def(pybind11::init<>());
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<None<T>, None<pybind11::object>>();
    }
  }
}

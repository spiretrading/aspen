export module Aspen:Concat;

import <string>;
import <type_traits>;
import <pybind11/pybind11.h>;
import :Concat;
#include "Aspen/Python/Reactor.hpp"

export namespace Aspen {

  /** Exports a Concat evaluating to a Python object. */
  void export_concat(pybind11::module& module);

  /**
   * Exports the generic Concat reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_concat(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Concat";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<Concat<T>>(module, name)
      .def(pybind11::init<T>());
    if constexpr(!std::is_same_v<T, SharedBox<SharedBox<pybind11::object>>>) {
      pybind11::implicitly_convertible<Concat<T>,
        Concat<SharedBox<SharedBox<pybind11::object>>>>();
    }
  }
}

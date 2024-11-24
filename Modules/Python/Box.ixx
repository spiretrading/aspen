module;
#include <string>
#include <type_traits>
#include <pybind11/pybind11.h>

export module Aspen.Python:Box;

import Aspen;
import :Reactor;

export namespace Aspen {

  /**
   * Exports the generic SharedBox reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_box(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Box";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<SharedBox<T>>(module, name).
      def(pybind11::init([] (pybind11::object reactor) {
        return to_python_reactor<T>(std::move(reactor));
      }));
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<
        SharedBox<T>, SharedBox<pybind11::object>>();
      pybind11::implicitly_convertible<
        SharedBox<pybind11::object>, SharedBox<T>>();
    }
    pybind11::implicitly_convertible<pybind11::object, SharedBox<T>>();
  }

  /** Exports a SharedBox evaluating to a Python object. */
  void export_box(pybind11::module& module) {
    export_box<pybind11::object>(module, "");
    export_box<void>(module, "None");
    pybind11::implicitly_convertible<
      SharedBox<pybind11::object>, SharedBox<void>>();
  }
}

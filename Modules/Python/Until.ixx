module;
#include <string>
#include <type_traits>
#include <pybind11/pybind11.h>

export module Aspen.Python:Until;

import Aspen;
import :Box;

export namespace Aspen {

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
    export_reactor<Until<C, T>>(module, name).
      def(pybind11::init<C, T>());
    if constexpr(!std::is_same_v<C, SharedBox<bool>> ||
        !std::is_same_v<T, SharedBox<pybind11::object>>) {
      pybind11::implicitly_convertible<Until<C, T>, Until<SharedBox<bool>,
        SharedBox<pybind11::object>>>();
    }
  }

  /** Exports an Until evaluating to a Python object. */
  void export_until(pybind11::module& module) {
    export_box<bool>(module, "Bool");
    export_until<SharedBox<bool>, SharedBox<pybind11::object>>(module, "");
    module.def("until",
      [] (SharedBox<bool> condition, SharedBox<pybind11::object> series) {
        return until(std::move(condition), std::move(series));
      });
  }
}

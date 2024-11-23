module;
#include <pybind11/pybind11.h>

export module Aspen.Python:Concat;

import <string>;
import <type_traits>;
import Aspen;
import :Box;

export namespace Aspen {

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
    export_reactor<Concat<T>>(module, name).
      def(pybind11::init<T>());
    if constexpr(!std::is_same_v<T, SharedBox<SharedBox<pybind11::object>>>) {
      pybind11::implicitly_convertible<
        Concat<T>, Concat<SharedBox<SharedBox<pybind11::object>>>>();
    }
  }

  /** Exports a Concat evaluating to a Python object. */
  void export_concat(pybind11::module& module) {
    export_box<SharedBox<pybind11::object>>(module, "Box");
    export_concat<SharedBox<SharedBox<pybind11::object>>>(module, "");
    module.def("concat", [] (SharedBox<SharedBox<pybind11::object>> producer) {
      return shared_box(concat(std::move(producer)));
    });
  }
}

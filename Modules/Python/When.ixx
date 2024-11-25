export module Aspen.Python:When;

import std;
import <pybind11/pybind11.h>;
import Aspen;
import :Box;

export namespace Aspen {

  /**
   * Exports the generic When reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename C, typename T>
  void export_when(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "When";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<When<C, T>>(module, name).
      def(pybind11::init<C, T>());
    if constexpr(!std::is_same_v<C, SharedBox<bool>> ||
        !std::is_same_v<T, SharedBox<pybind11::object>>) {
      pybind11::implicitly_convertible<
        When<C, T>, When<SharedBox<bool>, SharedBox<pybind11::object>>>();
    }
  }

  /** Exports a When evaluating to a Python object. */
  void export_when(pybind11::module& module) {
    export_box<bool>(module, "Bool");
    export_when<SharedBox<bool>, SharedBox<pybind11::object>>(module, "");
    module.def("when",
      [] (SharedBox<bool> condition, SharedBox<pybind11::object> series) {
        return when(std::move(condition), std::move(series));
      });
  }
}

export module Aspen.Python:Concur;

import std;
import <pybind11/pybind11.h>;
import Aspen;
import :Box;

export namespace Aspen {

  /**
   * Exports the generic Concur reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_concur(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Concur";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<Concur<T>>(module, name).
      def(pybind11::init<T>());
    if constexpr(!std::is_same_v<T, SharedBox<SharedBox<pybind11::object>>>) {
      pybind11::implicitly_convertible<
        Concur<T>, Concur<SharedBox<SharedBox<pybind11::object>>>>();
    }
  }

  /** Exports a Concur evaluating to a Python object. */
  void export_concur(pybind11::module& module) {
    export_box<SharedBox<pybind11::object>>(module, "Box");
    export_concur<SharedBox<SharedBox<pybind11::object>>>(module, "");
    module.def("concur", [] (SharedBox<SharedBox<pybind11::object>> producer) {
      return shared_box(concur(std::move(producer)));
    });
  }
}

module;
#include <pybind11/pybind11.h>

export module Aspen.Python:StateReactor;

import <string>;
import <type_traits>;
import Aspen;
import :Box;

export namespace Aspen {

  /**
   * Exports the generic StateReactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_state_reactor(
      pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "StateReactor";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<StateReactor<T>>(module, name).
      def(pybind11::init<T>());
    if constexpr(!std::is_same_v<T, SharedBox<pybind11::object>>) {
      pybind11::implicitly_convertible<
        StateReactor<T>, StateReactor<SharedBox<pybind11::object>>>();
    }
  }

  /** Exports a Python StateReactor. */
  void export_state_reactor(pybind11::module& module) {
    export_box<State>(module, "State");
    export_state_reactor<SharedBox<pybind11::object>>(module, "");
  }
}

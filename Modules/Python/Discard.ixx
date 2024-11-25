export module Aspen.Python:Discard;

import <pybind11/pybind11.h>;
import Aspen;
import :Box;

export namespace Aspen {

  /** Exports a Discard evaluating to a Python object. */
  void export_discard(pybind11::module& module) {
    export_box<bool>(module, "Bool");
    module.def("discard",
      [] (SharedBox<bool> toggle, SharedBox<pybind11::object> series) {
        return shared_box(discard(std::move(toggle), std::move(series)));
      });
  }
}

module;
#include <pybind11/pybind11.h>

export module Aspen.Python:Override;

import Aspen;
import :Box;

export namespace Aspen {

  /** Exports an Override evaluating to a Python object. */
  void export_override(pybind11::module& module) {
    export_box<SharedBox<pybind11::object>>(module, "Box");
    module.def("override",
      [] (SharedBox<SharedBox<pybind11::object>> producer) {
        return shared_box(override(std::move(producer)));
      });
  }
}

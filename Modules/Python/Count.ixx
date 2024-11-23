module;
#include <pybind11/pybind11.h>

export module Aspen.Python:Count;

import Aspen;
import :Box;

export namespace Aspen {

  /** Exports the count reactor. */
  void export_count(pybind11::module& module) {
    export_box<int>(module, "Int");
    module.def("count", [] (SharedBox<void> series) {
      return shared_box(count(std::move(series)));
    });
  }
}

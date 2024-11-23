module;
#include <pybind11/pybind11.h>

export module Aspen.Python:Last;

import Aspen;

export namespace Aspen {

  /** Exports the last reactor. */
  void export_last(pybind11::module& module) {
    module.def("last", [] (SharedBox<pybind11::object> source) {
      return shared_box(last(std::move(source)));
    });
  }
}

module;
#include <pybind11/pybind11.h>

export module Aspen.Python:Range;

import Aspen;

export namespace Aspen {

  /** Exports a range over Python objects. */
  void export_range(pybind11::module& module) {
    module.def("range",
      [] (SharedBox<pybind11::object> start, SharedBox<pybind11::object> stop) {
        return shared_box(range(
          std::move(start), std::move(stop), constant(pybind11::cast(1))));
      });
    module.def("range",
      [] (SharedBox<pybind11::object> start, SharedBox<pybind11::object> stop,
          SharedBox<pybind11::object> step) {
        return shared_box(
          range(std::move(start), std::move(stop), std::move(step)));
      });
  }
}

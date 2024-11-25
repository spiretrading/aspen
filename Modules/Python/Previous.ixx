export module Aspen.Python:Previous;

import <pybind11/pybind11.h>;
import Aspen;

export namespace Aspen {

  /** Exports a previous reactor evaluating to a Python object. */
  void export_previous(pybind11::module& module) {
    module.def("previous", [] (SharedBox<pybind11::object> source) {
      return shared_box(previous(std::move(source)));
    });
  }
}

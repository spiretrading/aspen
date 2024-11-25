export module Aspen.Python:First;

import <pybind11/pybind11.h>;
import Aspen;
import :Box;

export namespace Aspen {

  /** Exports the first reactor. */
  void export_first(pybind11::module& module) {
    module.def("first", [] (SharedBox<pybind11::object> source) {
      return shared_box(first(std::move(source)));
    });
  }
}

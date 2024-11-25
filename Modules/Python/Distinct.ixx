export module Aspen.Python:Distinct;

import <pybind11/pybind11.h>;
import Aspen;
import :Box;

template<>
struct std::hash<pybind11::object> {
  std::size_t operator()(const pybind11::object& o) const noexcept {
    return o.attr("__hash__")().cast<int>();
  }
};

template<>
struct std::equal_to<pybind11::object> {
  bool operator()(
      const pybind11::object& lhs, const pybind11::object& rhs) const noexcept {
    return lhs.equal(rhs);
  }
};

export namespace Aspen {

  /** Exports a distinct reactor evaluating to a Python object. */
  void export_distinct(pybind11::module& module) {
    module.def("discard", [] (SharedBox<pybind11::object> source) {
      return shared_box(distinct(std::move(source)));
    });
  }
}

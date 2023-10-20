#include "Aspen/Python/Distinct.hpp"
#include <pybind11/stl.h>
#include "Aspen/Distinct.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

template<>
struct std::hash<object> {
  std::size_t operator()(const object& o) const noexcept {
    return o.attr("__hash__")().cast<int>();
  }
};

template<>
struct std::equal_to<object> {
  bool operator()(const object& lhs, const object& rhs) const noexcept {
    return lhs.equal(rhs);
  }
};

void Aspen::export_distinct(pybind11::module& module) {
  module.def("discard", [] (SharedBox<object> source) {
    return shared_box(distinct(std::move(source)));
  });
}

#include "Aspen/Python/Distinct.hpp"
#include <pybind11/stl.h>
#include "Aspen/Distinct.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

template<>
struct std::hash<object> {
  std::size_t operator()(const object& o) const {
    return static_cast<std::size_t>(pybind11::hash(o));
  }
};

template<>
struct std::equal_to<object> {
  bool operator()(const object& lhs, const object& rhs) const {
    return lhs.equal(rhs);
  }
};

void Aspen::export_distinct(pybind11::module& module) {
  module.def("distinct", [] (SharedBox<object> source) {
    return shared_box(distinct(std::move(source)));
  });
}

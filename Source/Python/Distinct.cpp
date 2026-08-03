#include "Aspen/Python/Distinct.hpp"
#include <cstddef>
#include <pybind11/stl.h>
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

std::size_t Aspen::DistinctHash<object>::operator ()(
    const object& value) const {
  return static_cast<std::size_t>(hash(value));
}

bool Aspen::DistinctEquality<object>::operator ()(
    const object& left, const object& right) const {
  return left.equal(right);
}

void Aspen::export_distinct(pybind11::module& module) {
  module.def("distinct", [] (SharedBox<object> source) {
    return shared_box(distinct(std::move(source)));
  });
}

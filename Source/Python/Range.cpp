#include "Aspen/Python/Range.hpp"
#include "Aspen/Range.hpp"
#include "Aspen/Python/Constant.hpp"
#include "Aspen/Python/Object.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_range(pybind11::module& module) {
  module.def("range",
    [] (Box<object> start, Box<object> stop) {
      return Box(range(std::move(start), std::move(stop), constant(cast(1))));
    });
  module.def("range",
    [] (Box<object> start, Box<object> stop, Box<object> step) {
      return Box(range(std::move(start), std::move(stop), std::move(step)));
    });
}

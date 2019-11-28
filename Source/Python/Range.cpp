#include "Aspen/Python/Range.hpp"
#include "Aspen/Range.hpp"
#include "Aspen/Python/Constant.hpp"
#include "Aspen/Python/Object.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_range(pybind11::module& module) {
  module.def("range",
    [] (SharedBox<object> start, SharedBox<object> stop) {
      return shared_box(range(std::move(start), std::move(stop),
        constant(cast(1))));
    });
  module.def("range",
    [] (SharedBox<object> start, SharedBox<object> stop,
        SharedBox<object> step) {
      return shared_box(range(std::move(start), std::move(stop),
        std::move(step)));
    });
}

#include "Aspen/Python/Range.hpp"
#include "Aspen/Range.hpp"
#include "Aspen/Python/Box.hpp"
#include "Aspen/Python/Constant.hpp"
#include "Aspen/Python/Object.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_range(pybind11::module& module) {
  module.def("range",
    [] (object start, object stop) {
      return Box(range(to_python_reactor(std::move(start)),
        to_python_reactor(std::move(stop)), constant(cast(1))));
    });
}

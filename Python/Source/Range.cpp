#include "Aspen/Python/Range.hpp"
#include "Aspen/Python/Box.hpp"
#include "Aspen/Python/Constant.hpp"
#include "Aspen/Python/Object.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_range(pybind11::module& module) {
  module.def("range",
    [] (const std::shared_ptr<Box<object>>& start,
        const std::shared_ptr<Box<object>>& stop) {
      return Box(range(std::move(start), std::move(stop), constant(cast(1))));
    });
}

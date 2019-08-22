#include "Aspen/Python/Count.hpp"
#include "Aspen/Count.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_count(pybind11::module& module) {
  export_box<int>(module, "Int");
  module.def("count",
    [] (Box<void> series) {
      return Box(count(std::move(series)));
    });
}

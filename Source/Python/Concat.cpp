#include "Aspen/Python/Concat.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_concat(pybind11::module& module) {
  export_box<Box<object>>(module, "Box");
  export_concat<Box<Box<object>>>(module, "");
  module.def("concat",
    [] (Box<Box<object>> producer) {
      return Box(concat(std::move(producer)));
    });
}

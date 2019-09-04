#include "Aspen/Python/Override.hpp"
#include "Aspen/Override.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_override(pybind11::module& module) {
  export_box<Box<object>>(module, "Box");
  module.def("override",
    [] (Box<Box<object>> producer) {
      return Box(override(std::move(producer)));
    });
}

#include "Aspen/Python/Group.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_group(pybind11::module& module) {
  export_box<Box<object>>(module, "Box");
  export_group<Box<Box<object>>>(module, "");
  module.def("group",
    [] (Box<Box<object>> producer) {
      return Box(group(std::move(producer)));
    });
}

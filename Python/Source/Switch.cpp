#include "Aspen/Python/Switch.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_switch(pybind11::module& module) {
  export_box<bool>(module, "Bool");
  export_switch<Box<bool>, Box<object>>(module, "");
  module.def("switch",
    [] (Box<bool> toggle, Box<object> series) {
      return switch_(Box(std::move(toggle)), std::move(series));
    });
}

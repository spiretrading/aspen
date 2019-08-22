#include "Aspen/Python/Switch.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_switch(pybind11::module& module) {
  export_switch<Box<bool>, Box<object>>(module, "");
  module.def("switch",
    [] (object toggle, object series) {
      return switch_(Box(to_python_reactor<bool>(std::move(toggle))),
        to_python_reactor(std::move(series)));
    });
}

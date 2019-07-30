#include "Aspen/Python/Switch.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_switch(pybind11::module& module) {
  export_switch<Box<bool>, Box<object>>(module, "");
  module.def("switch", switch_<const Box<bool>&, const Box<object>&>);
}

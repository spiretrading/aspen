#include "Aspen/Python/None.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_none(module& module) {
  export_none<object>(module, "");
  module.def("none",
    [] {
      return none<object>();
    });
}

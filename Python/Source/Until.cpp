#include "Aspen/Python/Until.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_until(pybind11::module& module) {
  export_until<Box<bool>, Box<object>>(module, "");
  module.def("switch",
    [] (const Box<bool>& toggle, const Box<object>& series) {
      return until(toggle, series);
    });
}

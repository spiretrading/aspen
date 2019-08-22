#include "Aspen/Python/Until.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_until(pybind11::module& module) {
  export_until<Box<bool>, Box<object>>(module, "");
  module.def("until",
    [] (object toggle, object series) {
      return until(Box(to_python_reactor<bool>(std::move(toggle))),
        to_python_reactor(std::move(series)));
    });
}

#include "Aspen/Python/Until.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_until(pybind11::module& module) {
  export_until<Box<bool>, Box<object>>(module, "");
  module.def("until",
    [] (object toggle, Box<object> series) {
      return until(Box(PythonBox<bool>(std::move(toggle))), std::move(series));
    });
}

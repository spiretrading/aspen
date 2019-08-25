#include "Aspen/Python/Concat.hpp"
#include "Aspen/Python/PythonBox.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_concat(pybind11::module& module) {
  export_concat<Box<Box<object>>>(module, "");
  module.def("concat",
    [] (object producer) {
      return Box(concat(PythonBox<Box<object>>(producer)));
    });
}

#include "Aspen/Python/When.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_when(pybind11::module& module) {
  export_box<bool>(module, "Bool");
  export_when<Box<bool>, Box<object>>(module, "");
  module.def("when",
    [] (Box<bool> condition, Box<object> series) {
      return when(Box(std::move(condition)), std::move(series));
    });
}

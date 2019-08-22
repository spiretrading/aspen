#include "Aspen/Python/Until.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_until(pybind11::module& module) {
  export_box<bool>(module, "Bool");
  export_until<Box<bool>, Box<object>>(module, "");
  module.def("until",
    [] (Box<bool> toggle, Box<object> series) {
      return until(Box(std::move(toggle)), std::move(series));
    });
}

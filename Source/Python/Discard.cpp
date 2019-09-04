#include "Aspen/Python/Discard.hpp"
#include "Aspen/Discard.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_discard(pybind11::module& module) {
  export_box<bool>(module, "Bool");
  module.def("discard",
    [] (Box<bool> toggle, Box<object> series) {
      return Box(discard(std::move(toggle), std::move(series)));
    });
}

#include "Aspen/Python/Discard.hpp"
#include "Aspen/Discard.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_discard(pybind11::module& module) {
  module.def("discard",
    [] (object toggle, Box<object> series) {
      return Box(discard(Box(PythonBox<bool>(std::move(toggle))),
        std::move(series)));
    });
}

#include "Aspen/Python/Discard.hpp"
#include "Aspen/Discard.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_discard(pybind11::module& module) {
  module.def("discard",
    [] (object toggle, object series) {
      return Box(discard(to_python_reactor<bool>(std::move(toggle)),
        to_python_reactor(std::move(series))));
    });
}

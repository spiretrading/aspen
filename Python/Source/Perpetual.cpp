#include "Aspen/Python/Perpetual.hpp"
#include "Aspen/Perpetual.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_perpetual(module& module) {
  pybind11::class_<Perpetual>(module, "Perpetual")
    .def(init<>())
    .def("commit", &Perpetual::commit)
    .def("eval", &Perpetual::eval);
  implicitly_convertible_to_box<Perpetual>();
  module.def("perpetual", perpetual);
}

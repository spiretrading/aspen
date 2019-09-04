#include "Aspen/Python/Perpetual.hpp"
#include "Aspen/Perpetual.hpp"
#include "Aspen/Python/Box.hpp"
#include "Aspen/Python/Reactor.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_perpetual(module& module) {
  export_reactor<Perpetual>(module, "Perpetual")
    .def(init<>());
  module.def("perpetual", perpetual);
}

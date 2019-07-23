#include "Aspen/Python/Executor.hpp"
#include "Aspen/Executor.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_executor(pybind11::module& module) {
  class_<Executor>(module, "Executor")
    .def(init<Box<void>>())
    .def("run_until_none", &Executor::run_until_none)
    .def("run_until_complete", &Executor::run_until_complete);
}

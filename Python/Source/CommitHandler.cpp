#include "Aspen/Python/CommitHandler.hpp"
#include <pybind11/stl.h>
#include "Aspen/CommitHandler.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_commit_handler(pybind11::module& module) {
  class_<CommitHandler>(module, "CommitHandler")
    .def(init<std::vector<Box<void>>>())
    .def("commit", &CommitHandler::commit)
    .def("transfer", &CommitHandler::transfer);
}

#include "Aspen/Python/CommitFlag.hpp"
#include "Aspen/CommitFlag.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_commit_flag(pybind11::module& module) {
  class_<CommitFlag>(module, "CommitFlag")
    .def_static("get_current", &CommitFlag::get_current,
      return_value_policy::reference)
    .def(init<>())
    .def_property_readonly("is_raised", &CommitFlag::is_raised)
    .def("raise_", &CommitFlag::raise)
    .def("clear", &CommitFlag::clear);
}

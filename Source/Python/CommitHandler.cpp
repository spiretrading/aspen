#include "Aspen/Python/CommitHandler.hpp"
#include <pybind11/stl.h>
#include "Aspen/CommitHandler.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_commit_handler(pybind11::module& module) {
  using Handler = CommitHandler<SharedBox<object>>;
  class_<Handler>(module, "CommitHandler")
    .def(init<std::vector<SharedBox<object>>>())
    .def("__len__", &Handler::size)
    .def("__getitem__",
      [] (Handler& self, std::size_t index) {
        if(index >= self.size()) {
          throw index_error();
        }
        return self.get(index);
      })
    .def_property_readonly("evaluated", &Handler::get_evaluated)
    .def("commit", &Handler::commit);
}

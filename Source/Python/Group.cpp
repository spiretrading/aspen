#include "Aspen/Python/Group.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_group(pybind11::module& module) {
  export_box<SharedBox<object>>(module, "Box");
  export_group<SharedBox<SharedBox<object>>>(module, "");
  module.def("group",
    [] (SharedBox<SharedBox<object>> producer) {
      return SharedBox(group(std::move(producer)));
    });
}

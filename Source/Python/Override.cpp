#include "Aspen/Python/Override.hpp"
#include "Aspen/Override.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_override(pybind11::module& module) {
  export_box<SharedBox<object>>(module, "Box");
  module.def("override",
    [] (SharedBox<SharedBox<object>> producer) {
      return SharedBox(override(std::move(producer)));
    });
}

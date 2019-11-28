#include "Aspen/Python/Concat.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_concat(pybind11::module& module) {
  export_box<SharedBox<object>>(module, "Box");
  export_concat<SharedBox<SharedBox<object>>>(module, "");
  module.def("concat",
    [] (SharedBox<SharedBox<object>> producer) {
      return SharedBox(concat(std::move(producer)));
    });
}

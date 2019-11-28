#include "Aspen/Python/When.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_when(pybind11::module& module) {
  export_box<bool>(module, "Bool");
  export_when<SharedBox<bool>, SharedBox<object>>(module, "");
  module.def("when",
    [] (SharedBox<bool> condition, SharedBox<object> series) {
      return when(std::move(condition), std::move(series));
    });
}

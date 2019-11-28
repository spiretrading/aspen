#include "Aspen/Python/Until.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_until(pybind11::module& module) {
  export_box<bool>(module, "Bool");
  export_until<SharedBox<bool>, SharedBox<object>>(module, "");
  module.def("until",
    [] (SharedBox<bool> condition, SharedBox<object> series) {
      return until(std::move(condition), std::move(series));
    });
}

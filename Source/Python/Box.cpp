#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_box(pybind11::module& module) {
  export_box<object>(module, "");
  export_box<void>(module, "None");
  implicitly_convertible<SharedBox<object>, SharedBox<void>>();
}

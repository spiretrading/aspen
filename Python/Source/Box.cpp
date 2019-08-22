#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

bool Aspen::is_python_reactor(const object& value) {
  return hasattr(value, "commit") &&
    PyCallable_Check(value.attr("commit").ptr()) &&
    hasattr(value, "eval") && PyCallable_Check(value.attr("eval").ptr());
}

void Aspen::export_box(pybind11::module& module) {
  export_box<object>(module, "");
  export_box<void>(module, "None");
  implicitly_convertible<Box<object>, Box<void>>();
}

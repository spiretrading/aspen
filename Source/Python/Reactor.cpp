#include "Aspen/Python/Reactor.hpp"

using namespace Aspen;
using namespace pybind11;

bool Aspen::is_python_reactor(const object& value) {
  return hasattr(value, "commit") &&
    PyCallable_Check(value.attr("commit").ptr()) &&
    hasattr(value, "eval") && PyCallable_Check(value.attr("eval").ptr());
}

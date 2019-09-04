#include "Aspen/Python/Queue.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_queue(pybind11::module& module) {
  export_queue<object>(module, "");
}

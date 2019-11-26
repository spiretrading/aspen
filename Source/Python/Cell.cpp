#include "Aspen/Python/Cell.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_cell(pybind11::module& module) {
  export_cell<object>(module, "");
}

#include <pybind11/pybind11.h>
#include "Aspen/Python/Constant.hpp"

using namespace Aspen;
using namespace pybind11;

PYBIND11_MODULE(aspen, module) {
  export_constant(module);
}

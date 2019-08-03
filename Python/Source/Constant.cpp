#include "Aspen/Python/Constant.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_constant(module& module) {
  export_constant<object>(module, "");
  module.def("constant",
    [] (object value) {
      return constant(std::move(value));
    });
}

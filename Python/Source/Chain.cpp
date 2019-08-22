#include "Aspen/Python/Chain.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_chain(pybind11::module& module) {
  export_chain<Box<object>, Box<object>>(module, "");
  module.def("chain",
    [] (object a, object b) {
      return chain(to_python_reactor(std::move(a)),
        to_python_reactor(std::move(b)));
    });
}

#include "Aspen/Python/Proxy.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_proxy(pybind11::module& module) {
  export_proxy<SharedBox<object>>(module, "");
  module.def("proxy",
    [] {
      return proxy<SharedBox<object>>();
    });
}

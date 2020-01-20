#include "Aspen/Python/Concur.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_concur(pybind11::module& module) {
  export_box<SharedBox<object>>(module, "Box");
  export_concur<SharedBox<SharedBox<object>>>(module, "");
  module.def("concur",
    [] (SharedBox<SharedBox<object>> producer) {
      return shared_box(concur(std::move(producer)));
    });
}

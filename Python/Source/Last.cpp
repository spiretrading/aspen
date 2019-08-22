#include "Aspen/Python/Last.hpp"
#include "Aspen/Last.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_last(pybind11::module& module) {
  module.def("last",
    [] (object source) {
      return Box(last(to_python_reactor(std::move(source))));
    });
}

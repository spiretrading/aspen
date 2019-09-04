#include "Aspen/Python/First.hpp"
#include "Aspen/First.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_first(pybind11::module& module) {
  module.def("first",
    [] (Box<object> source) {
      return Box(first(std::move(source)));
    });
}

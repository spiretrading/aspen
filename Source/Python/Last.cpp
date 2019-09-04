#include "Aspen/Python/Last.hpp"
#include "Aspen/Last.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_last(pybind11::module& module) {
  module.def("last",
    [] (Box<object> source) {
      return Box(last(std::move(source)));
    });
}

#include "Aspen/Python/Unconsecutive.hpp"
#include "Aspen/Unconsecutive.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_unconsecutive(pybind11::module& module) {
  module.def("unconsecutive",
    [] (SharedBox<object> series) {
      return SharedBox(unconsecutive(std::move(series)));
    });
}

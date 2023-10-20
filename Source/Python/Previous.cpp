#include "Aspen/Python/Previous.hpp"
#include "Aspen/Previous.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_previous(pybind11::module& module) {
  module.def("previous", [] (SharedBox<object> source) {
    return shared_box(previous(std::move(source)));
  });
}

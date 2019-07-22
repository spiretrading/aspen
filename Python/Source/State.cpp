#include "Aspen/Python/State.hpp"
#include "Aspen/State.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_state(pybind11::module& module) {
  enum_<State>(module, "State")
    .value("NONE", State::NONE)
    .value("EVALUATED", State::EVALUATED)
    .value("COMPLETE", State::COMPLETE)
    .value("COMPLETE_EMPTY", State::COMPLETE_EMPTY)
    .value("COMPLETE_EVALUATED", State::COMPLETE_EVALUATED);
  module.def("is_complete", is_complete);
  module.def("has_evaluation", has_evaluation);
}

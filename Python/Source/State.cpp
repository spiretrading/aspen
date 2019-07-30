#include "Aspen/Python/State.hpp"
#include "Aspen/State.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_state(pybind11::module& module) {
  enum_<State>(module, "State")
    .value("NONE", State::NONE)
    .value("EMPTY", State::EMPTY)
    .value("EVALUATED", State::EVALUATED)
    .value("CONTINUE", State::CONTINUE)
    .value("CONTINUE_EMPTY", State::CONTINUE_EMPTY)
    .value("CONTINUE_EVALUATED", State::CONTINUE_EVALUATED)
    .value("COMPLETE", State::COMPLETE)
    .value("COMPLETE_EMPTY", State::COMPLETE_EMPTY)
    .value("COMPLETE_EVALUATED", State::COMPLETE_EVALUATED);
  module.def("combine", combine);
  module.def("has_evaluation", has_evaluation);
  module.def("is_empty", is_empty);
  module.def("has_continuation", has_continuation);
  module.def("is_complete", is_complete);
  module.def("set", set);
  module.def("reset", reset);
}

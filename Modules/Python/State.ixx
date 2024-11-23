module;
#include <pybind11/pybind11.h>

export module Aspen.Python:State;

import Aspen;

export namespace Aspen {

  /** Exports the State enum and associated functions. */
  void export_state(pybind11::module& module) {
    pybind11::enum_<State>(module, "State").
      value("NONE", State::NONE).
      value("EVALUATED", State::EVALUATED).
      value("CONTINUE", State::CONTINUE).
      value("CONTINUE_EVALUATED", State::CONTINUE_EVALUATED).
      value("COMPLETE", State::COMPLETE).
      value("COMPLETE_EVALUATED", State::COMPLETE_EVALUATED);
    module.def("combine", combine);
    module.def("has_evaluation", has_evaluation);
    module.def("has_continuation", has_continuation);
    module.def("is_complete", is_complete);
    module.def("set", set);
    module.def("reset", reset);
  }
}

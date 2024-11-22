export module Aspen:State;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports the State enum and associated functions. */
  void export_state(pybind11::module& module);
}

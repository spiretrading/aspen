export module Aspen:Lift;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports the Lift class. */
  void export_lift(pybind11::module& module);
}

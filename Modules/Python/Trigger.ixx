export module Aspen:Trigger;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports the Trigger class. */
  void export_trigger(pybind11::module& module);
}

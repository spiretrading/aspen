export module Aspen:Override;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports an Override evaluating to a Python object. */
  void export_override(pybind11::module& module);
}

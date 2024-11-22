export module Aspen:Distinct;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports a distinct reactor evaluating to a Python object. */
  void export_distinct(pybind11::module& module);
}

export module Aspen:Previous;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports a previous reactor evaluating to a Python object. */
  void export_previous(pybind11::module& module);
}

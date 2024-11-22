export module Aspen:Discard;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports a Discard evaluating to a Python object. */
  void export_discard(pybind11::module& module);
}

export module Aspen:Unconsecutive;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports the unconsecutive reactor to Python. */
  void export_unconsecutive(pybind11::module& module);
}

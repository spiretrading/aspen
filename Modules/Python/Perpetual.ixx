export module Aspen:Perpetual;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports the Perpetual class. */
  void export_perpetual(pybind11::module& module);
}

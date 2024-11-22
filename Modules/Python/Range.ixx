export module Aspen:Range;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports a range over Python objects. */
  void export_range(pybind11::module& module);
}

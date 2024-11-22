export module Aspen:Count;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports the count reactor. */
  void export_count(pybind11::module& module);
}

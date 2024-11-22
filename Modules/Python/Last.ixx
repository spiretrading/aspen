export module Aspen:Last;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports the last reactor. */
  void export_last(pybind11::module& module);
}

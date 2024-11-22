export module Aspen:First;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports the first reactor. */
  void export_first(pybind11::module& module);
}

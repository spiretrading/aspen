export module Aspen:Executor;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports the Executor class. */
  void export_executor(pybind11::module& module);
}

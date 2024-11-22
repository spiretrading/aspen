export module Aspen:CommitHandler;

import <pybind11/pybind11.h>;

export namespace Aspen {

  /** Exports the CommitHandler class. */
  void export_commit_handler(pybind11::module& module);
}

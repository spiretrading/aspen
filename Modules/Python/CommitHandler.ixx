module;
#include <pybind11/pybind11.h>

export module Aspen.Python:CommitHandler;

import Aspen;

export namespace Aspen {

  /** Exports the CommitHandler class. */
  void export_commit_handler(pybind11::module& module) {
    pybind11::class_<CommitHandler<SharedBox<pybind11::object>>>(
        module, "CommitHandler").
      def(pybind11::init<std::vector<SharedBox<pybind11::object>>>()).
      def("commit", &CommitHandler<SharedBox<pybind11::object>>::commit);
  }
}

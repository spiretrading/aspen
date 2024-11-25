export module Aspen.Python:Executor;

import <pybind11/pybind11.h>;
import Aspen;
import :GilAcquireReactor;

export namespace Aspen {

  /** Exports the Executor class. */
  void export_executor(pybind11::module& module) {
    pybind11::class_<Executor>(module, "Executor").
      def(pybind11::init([] (SharedBox<void> reactor) {
        return std::make_unique<Executor>(
          GilAcquireReactor(std::move(reactor)));
      })).
      def("run_until_none", &Executor::run_until_none).
      def("run_until_complete", &Executor::run_until_complete,
        pybind11::call_guard<pybind11::gil_scoped_release>());
  }
}

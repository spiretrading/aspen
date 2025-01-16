export module Aspen.Python:Perpetual;

import <pybind11/pybind11.h>;
import Aspen;
import :Reactor;

export namespace Aspen {

  /** Exports the Perpetual class. */
  void export_perpetual(pybind11::module& module) {
    export_reactor<Perpetual>(module, "Perpetual").
      def(pybind11::init<>());
    module.def("perpetual", perpetual);
  }
}
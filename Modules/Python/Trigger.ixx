module;
#include <pybind11/pybind11.h>

export module Aspen.Python:Trigger;

import Aspen;

export namespace Aspen {

  /** Exports the Trigger class. */
  void export_trigger(pybind11::module& module) {
    pybind11::class_<Trigger>(module, "Trigger").
      def_static("get_trigger", &Trigger::get_trigger,
        pybind11::return_value_policy::reference).
      def_static("set_trigger",
        static_cast<void (*)(Trigger*)>(&Trigger::set_trigger)).
      def(pybind11::init<>()).
      def(pybind11::init<Trigger::Slot>()).
      def("signal", &Trigger::signal);
  }
}

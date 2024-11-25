export module Aspen.Python:Queue;

import std;
import <pybind11/pybind11.h>;
import Aspen;
import :Reactor;

export namespace Aspen {

  /**
   * Exports the generic Queue class.
   * @param module The module to export the class to.
   * @param prefix The name of the type the Queue generates.
   */
  template<typename T>
  void export_queue(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + std::string("Queue");
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<Queue<T>>(module, name).
      def(pybind11::init<>()).
      def("push", &Queue<T>::push).
      def("set_complete",
        static_cast<void (Queue<T>::*)()>(&Queue<T>::set_complete)).
      def("set_complete",
        static_cast<void (Queue<T>::*)(T)>(&Queue<T>::set_complete));
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<Queue<T>, Queue<pybind11::object>>();
    }
  }

  /** Exports a Queue evaluating to a Python object. */
  void export_queue(pybind11::module& module) {
    export_queue<pybind11::object>(module, "");
  }
}

#ifndef ASPEN_PYTHON_QUEUE_HPP
#define ASPEN_PYTHON_QUEUE_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Aspen/Queue.hpp"
#include "Aspen/Python/Box.hpp"

namespace Aspen {

  /** Exports a Queue evaluating to a Python object. */
  void export_queue(pybind11::module& module);

  /**
   * Exports the generic Queue class.
   * @param module The module to export the class to.
   * @param prefix The name of the type the Queue generates.
   */
  template<typename T>
  void export_queue(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + std::string("Queue");
    pybind11::class_<Queue<T>>(module, name.c_str())
      .def(pybind11::init<>())
      .def("push", &Queue<T>::push)
      .def("set_complete",
        static_cast<void (Queue<T>::*)()>(&Queue<T>::set_complete))
      .def("set_complete",
        static_cast<void (Queue<T>::*)(T)>(&Queue<T>::set_complete))
      .def("commit", &Queue<T>::commit)
      .def("eval", &Queue<T>::eval);
    implicitly_convertible_to_box<Queue<T>>();
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<Queue<T>,
        Queue<pybind11::object>>();
    }
  }
}

#endif

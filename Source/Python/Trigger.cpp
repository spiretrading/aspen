#include "Aspen/Python/Trigger.hpp"
#include <memory>
#include <utility>
#include <pybind11/stl.h>
#include "Aspen/Trigger.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_trigger(pybind11::module& module) {
  class_<Trigger>(module, "Trigger")
    .def_static("get_trigger", &Trigger::get_trigger,
      return_value_policy::reference)
    .def_static("set_trigger",
      static_cast<void (*)(Trigger*)>(&Trigger::set_trigger))
    .def(init<>())
    .def(init(
      [] (object slot) {
        return std::make_unique<Trigger>([slot = std::move(slot)] {
          auto lock = gil_scoped_acquire();
          try {
            slot();
          } catch(error_already_set& error) {
            error.restore();
            PyErr_Print();
          } catch(...) {}
        });
      }))
    .def("signal", &Trigger::signal);
}

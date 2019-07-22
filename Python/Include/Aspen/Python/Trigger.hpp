#ifndef ASPEN_PYTHON_TRIGGER_HPP
#define ASPEN_PYTHON_TRIGGER_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports the Trigger class. */
  void export_trigger(pybind11::module& module);
}

#endif

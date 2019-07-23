#ifndef ASPEN_PYTHON_LIFT_HPP
#define ASPEN_PYTHON_LIFT_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports the Lift class. */
  void export_lift(pybind11::module& module);
}

#endif

#ifndef ASPEN_PYTHON_OVERRIDE_HPP
#define ASPEN_PYTHON_OVERRIDE_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports an Override evaluating to a Python object. */
  void export_override(pybind11::module& module);
}

#endif

#ifndef ASPEN_PYTHON_PREVIOUS_HPP
#define ASPEN_PYTHON_PREVIOUS_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports a previous reactor evaluating to a Python object. */
  void export_previous(pybind11::module& module);
}

#endif

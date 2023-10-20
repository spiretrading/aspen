#ifndef ASPEN_PYTHON_DISTINCT_HPP
#define ASPEN_PYTHON_DISTINCT_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports a distinct reactor evaluating to a Python object. */
  void export_distinct(pybind11::module& module);
}

#endif

#ifndef ASPEN_PYTHON_RANGE_HPP
#define ASPEN_PYTHON_RANGE_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports a range over Python objects. */
  void export_range(pybind11::module& module);
}

#endif

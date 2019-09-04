#ifndef ASPEN_PYTHON_COUNT_HPP
#define ASPEN_PYTHON_COUNT_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports the count reactor. */
  void export_count(pybind11::module& module);
}

#endif

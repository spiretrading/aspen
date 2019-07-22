#ifndef ASPEN_PYTHON_FIRST_HPP
#define ASPEN_PYTHON_FIRST_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports the first reactor. */
  void export_first(pybind11::module& module);
}

#endif

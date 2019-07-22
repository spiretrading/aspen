#ifndef ASPEN_PYTHON_LAST_HPP
#define ASPEN_PYTHON_LAST_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports the last reactor. */
  void export_last(pybind11::module& module);
}

#endif

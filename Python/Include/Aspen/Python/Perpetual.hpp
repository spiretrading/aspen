#ifndef ASPEN_PYTHON_PERPETUAL_HPP
#define ASPEN_PYTHON_PERPETUAL_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports the Perpetual class. */
  void export_perpetual(pybind11::module& module);
}

#endif

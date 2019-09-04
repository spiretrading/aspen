#ifndef ASPEN_PYTHON_UNCONSECUTIVE_HPP
#define ASPEN_PYTHON_UNCONSECUTIVE_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports the unconsecutive reactor to Python. */
  void export_unconsecutive(pybind11::module& module);
}

#endif

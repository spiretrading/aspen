#ifndef ASPEN_PYTHON_STATE_HPP
#define ASPEN_PYTHON_STATE_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports the State enum and associated functions. */
  void export_state(pybind11::module& module);
}

#endif

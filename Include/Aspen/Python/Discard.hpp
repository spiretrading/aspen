#ifndef ASPEN_PYTHON_DISCARD_HPP
#define ASPEN_PYTHON_DISCARD_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports a Discard evaluating to a Python object. */
  void export_discard(pybind11::module& module);
}

#endif

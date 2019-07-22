#ifndef ASPEN_PYTHON_RANGE_HPP
#define ASPEN_PYTHON_RANGE_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Aspen/Box.hpp"
#include "Aspen/Range.hpp"

namespace Aspen {

  /** Exports a range over Python objects. */
  void export_range(pybind11::module& module);
}

#endif

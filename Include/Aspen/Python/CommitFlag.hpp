#ifndef ASPEN_PYTHON_COMMIT_FLAG_HPP
#define ASPEN_PYTHON_COMMIT_FLAG_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports the CommitFlag class. */
  void export_commit_flag(pybind11::module& module);
}

#endif

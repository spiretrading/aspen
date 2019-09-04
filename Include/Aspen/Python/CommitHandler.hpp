#ifndef ASPEN_PYTHON_COMMIT_HANDLER_HPP
#define ASPEN_PYTHON_COMMIT_HANDLER_HPP
#include <pybind11/pybind11.h>

namespace Aspen {

  /** Exports the CommitHandler class. */
  void export_commit_handler(pybind11::module& module);
}

#endif

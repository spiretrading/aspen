#ifndef ASPEN_PYTHON_OBJECT_HPP
#define ASPEN_PYTHON_OBJECT_HPP
#include <pybind11/pybind11.h>
#include "Aspen/Python/DllExports.hpp"

namespace pybind11 {
  ASPEN_EXPORT_DLL object operator %(const object& left, const object& right);

  ASPEN_EXPORT_DLL object operator +(const object& value);
}

#endif

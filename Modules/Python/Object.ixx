export module Aspen:Object;

import <pybind11/pybind11.h>;
#include "Aspen/Python/DllExports.hpp"

namespace pybind11 {
  ASPEN_EXPORT_DLL object operator %(const object& left, const object& right);

  ASPEN_EXPORT_DLL object operator +(const object& value);
}

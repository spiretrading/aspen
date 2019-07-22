#ifndef ASPEN_PYTHON_OBJECT_HPP
#define ASPEN_PYTHON_OBJECT_HPP
#include <pybind11/pybind11.h>

namespace pybind11 {
  object operator +(const object& left, const object& right);

  bool operator <(const object& left, const object& right);

  bool operator >=(const object& left, const object& right);
}

#endif

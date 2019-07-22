#ifndef ASPEN_PYTHON_OBJECT_HPP
#define ASPEN_PYTHON_OBJECT_HPP
#include <pybind11/pybind11.h>

namespace pybind11 {
  object operator +(const object& left, const object& right) {
    return left.attr("__add__")(right);
  }

  bool operator <(const object& left, const object& right) {
    return left.attr("__lt__")(right).cast<bool>();
  }

  bool operator >=(const object& left, const object& right) {
    return left.attr("__ge__")(right).cast<bool>();
  }
}

#endif

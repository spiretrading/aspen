#include "Aspen/Python/Object.hpp"

using namespace pybind11;

object pybind11::operator +(const object& left, const object& right) {
  auto r = left.attr("__add__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__radd__")(left);
  }
  return r;
}

bool pybind11::operator <(const object& left, const object& right) {
  auto r = left.attr("__lt__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__ge__")(left).cast<bool>();
  }
  return r.cast<bool>();
}

bool pybind11::operator >=(const object& left, const object& right) {
  auto r = left.attr("__ge__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__lt__")(left).cast<bool>();
  }
  return r.cast<bool>();
}

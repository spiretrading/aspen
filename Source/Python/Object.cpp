#include "Aspen/Python/Object.hpp"

using namespace pybind11;

object pybind11::operator %(const object& left, const object& right) {
  auto r = left.attr("__mod__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__rmod__")(left);
  }
  return r;
}

object pybind11::operator +(const object& value) {
  return value.attr("__pos__")();
}

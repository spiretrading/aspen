#include "Aspen/Python/Object.hpp"

using namespace pybind11;

object pybind11::operator +(const object& left, const object& right) {
  auto r = left.attr("__add__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__radd__")(left);
  }
  return r;
}

object pybind11::operator -(const object& left, const object& right) {
  auto r = left.attr("__sub__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__rsub__")(left);
  }
  return r;
}

object pybind11::operator *(const object& left, const object& right) {
  auto r = left.attr("__mul__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__rmul__")(left);
  }
  return r;
}

object pybind11::operator /(const object& left, const object& right) {
  auto r = left.attr("__truediv__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__rtruediv__")(left);
  }
  return r;
}

object pybind11::operator %(const object& left, const object& right) {
  auto r = left.attr("__mod__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__rmod__")(left);
  }
  return r;
}

object pybind11::operator ^(const object& left, const object& right) {
  auto r = left.attr("__xor__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__rxor__")(left);
  }
  return r;
}

object pybind11::operator &(const object& left, const object& right) {
  auto r = left.attr("__and__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__rand__")(left);
  }
  return r;
}

object pybind11::operator |(const object& left, const object& right) {
  auto r = left.attr("__or__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__ror__")(left);
  }
  return r;
}

object pybind11::operator ~(const object& value) {
  return value.attr("__invert__")();
}

object pybind11::operator <<(const object& left, const object& right) {
  auto r = left.attr("__lshift__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__rlshift__")(left);
  }
  return r;
}

object pybind11::operator >>(const object& left, const object& right) {
  auto r = left.attr("__rshift__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__rrshift__")(left);
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

bool pybind11::operator <=(const object& left, const object& right) {
  auto r = left.attr("__le__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__gt__")(left).cast<bool>();
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

bool pybind11::operator >(const object& left, const object& right) {
  auto r = left.attr("__gt__")(right);
  if(r.ptr() == Py_NotImplemented) {
    return right.attr("__le__")(left).cast<bool>();
  }
  return r.cast<bool>();
}

object pybind11::operator -(const object& value) {
  return value.attr("__neg__")();
}

object pybind11::operator +(const object& value) {
  return value.attr("__pos__")();
}

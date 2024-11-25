module;
#include "Aspen/Python/DllExports.hpp"

export module Aspen.Python:Object;

import <pybind11/pybind11.h>;

export namespace pybind11 {
  ASPEN_EXPORT_DLL object operator %(const object& left, const object& right) {
    auto r = left.attr("__mod__")(right);
    if(r.ptr() == Py_NotImplemented) {
      return right.attr("__rmod__")(left);
    }
    return r;
  }

  ASPEN_EXPORT_DLL object operator +(const object& value) {
    return value.attr("__pos__")();
  }
}

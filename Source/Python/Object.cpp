#include "Aspen/Python/Object.hpp"

using namespace pybind11;

object pybind11::operator %(const object& left, const object& right) {
  if(auto result = PyNumber_Remainder(left.ptr(), right.ptr())) {
    return reinterpret_steal<object>(result);
  }
  throw error_already_set();
}

object pybind11::operator +(const object& value) {
  if(auto result = PyNumber_Positive(value.ptr())) {
    return reinterpret_steal<object>(result);
  }
  throw error_already_set();
}

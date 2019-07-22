#include "Aspen/Python/Object.hpp"

using namespace pybind11;

object pybind11::operator +(const object& left, const object& right) {
  return left.attr("__add__")(right);
}

bool pybind11::operator <(const object& left, const object& right) {
  return left.attr("__lt__")(right).cast<bool>();
}

bool pybind11::operator >=(const object& left, const object& right) {
  return left.attr("__ge__")(right).cast<bool>();
}

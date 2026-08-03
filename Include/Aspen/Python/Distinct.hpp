#ifndef ASPEN_PYTHON_DISTINCT_HPP
#define ASPEN_PYTHON_DISTINCT_HPP
#include <cstddef>
#include <pybind11/pybind11.h>
#include "Aspen/Distinct.hpp"
#include "Aspen/Python/DllExports.hpp"

namespace Aspen {

  /** Exports a distinct reactor evaluating to a Python object. */
  void export_distinct(pybind11::module& module);

  template<>
  struct DistinctHash<pybind11::object> {
    ASPEN_EXPORT_DLL std::size_t operator ()(
      const pybind11::object& value) const;
  };

  template<>
  struct DistinctEquality<pybind11::object> {
    ASPEN_EXPORT_DLL bool operator ()(
      const pybind11::object& left, const pybind11::object& right) const;
  };
}

#endif

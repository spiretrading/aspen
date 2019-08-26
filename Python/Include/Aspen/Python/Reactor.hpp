#ifndef ASPEN_PYTHON_REACTOR_HPP
#define ASPEN_PYTHON_REACTOR_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Aspen/Box.hpp"
#include "Aspen/Operators.hpp"
#include "Aspen/Traits.hpp"
#include "Aspen/Python/Object.hpp"
#include "Aspen/Python/PythonBox.hpp"

namespace Aspen {

  /** Tests if a Python object represents a reactor. */
  bool is_python_reactor(const pybind11::object& value);

  /** Wraps a Python object into an appropriate reactor. */
  template<typename T = pybind11::object>
  Box<T> to_python_reactor(pybind11::object value) {
    if(is_python_reactor(value)) {
      return Box(PythonBox<T>(std::move(value)));
    } else {
      if constexpr(std::is_same_v<T, void>) {
        return Box<void>(value);
      } else if constexpr(std::is_same_v<T, Box<pybind11::object>>) {
        return Box(PythonBox<T>(std::move(value)));
      } else {
        return Box(value.cast<T>());
      }
    }
  }

  /** Exports implicit conversions from a reactor to Box types. */
  template<typename T>
  void implicitly_convertible_to_box() {
    pybind11::implicitly_convertible<T, Box<reactor_result_t<T>>>();
    pybind11::implicitly_convertible<T, Box<void>>();
    if constexpr(!std::is_same_v<reactor_result_t<T>, pybind11::object>) {
      pybind11::implicitly_convertible<T, Box<pybind11::object>>();
    }
  }

  /**
   * Exports a reactor type to Python.
   * @param module The module to export the reactor to.
   * @param name The name of the type.
   * @return The exported reactor type.
   */
  template<typename T, typename... Options>
  auto export_reactor(pybind11::module& module, const std::string& name) {
    using Type = reactor_result_t<T>;
    auto reactor = pybind11::class_<T, Options...>(module, name.c_str())
      .def("commit", &T::commit)
      .def("eval", &T::eval);
    if constexpr(!std::is_same_v<Type, void>) {
      reactor.def("__add__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) +
            to_python_reactor(object));
        });
      reactor.def("__sub__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) -
            to_python_reactor(object));
        });
      reactor.def("__mul__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) *
            to_python_reactor(object));
        });
      reactor.def("__truediv__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) /
            to_python_reactor(object));
        });
      reactor.def("__mod__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) %
            to_python_reactor(object));
        });
      reactor.def("__xor__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) ^
            to_python_reactor(object));
        });
      reactor.def("__and__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) &
            to_python_reactor(object));
        });
      reactor.def("__or__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) |
            to_python_reactor(object));
        });
      reactor.def("__invert__",
        [] (const pybind11::object& self) {
          return Box(~PythonBox<pybind11::object>(self));
        });
      reactor.def("__lshift__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) <<
            to_python_reactor(object));
        });
      reactor.def("__rshift__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) >>
            to_python_reactor(object));
        });
      reactor.def("__lt__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) <
            to_python_reactor(object));
        });
      reactor.def("__le__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) <=
            to_python_reactor(object));
        });
      reactor.def("__ge__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) >=
            to_python_reactor(object));
        });
      reactor.def("__gt__",
        [] (const pybind11::object& self, const pybind11::object& object) {
          return Box(PythonBox<pybind11::object>(self) >
            to_python_reactor(object));
        });
      reactor.def("__neg__",
        [] (const pybind11::object& self) {
          return Box(-PythonBox<pybind11::object>(self));
        });
      reactor.def("__pos__",
        [] (const pybind11::object& self) {
          return Box(+PythonBox<pybind11::object>(self));
        });
    }
    if constexpr(!std::is_same_v<T, Box<Type>>) {
      implicitly_convertible_to_box<T>();
    }
    return reactor;
  }
}

#endif

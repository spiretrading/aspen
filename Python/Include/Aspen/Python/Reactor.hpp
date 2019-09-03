#ifndef ASPEN_PYTHON_REACTOR_HPP
#define ASPEN_PYTHON_REACTOR_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Aspen/Box.hpp"
#include "Aspen/Operators.hpp"
#include "Aspen/Traits.hpp"
#include "Aspen/Unique.hpp"
#include "Aspen/Python/Object.hpp"
#include "Aspen/Python/PythonBox.hpp"

PYBIND11_DECLARE_HOLDER_TYPE(T, Aspen::Shared<Aspen::Unique<T>>);

namespace pybind11::detail {
  template <typename T>
  struct holder_helper<Aspen::Shared<Aspen::Unique<T>>> {
    static const T* get(const Aspen::Shared<Aspen::Unique<T>>& reactor) {
      return &**reactor;
    }
  };
}

namespace Aspen {

  /**
   * The type used to ensure that all reactors are properly shared from within
   * Python.
   */
  template<typename T>
  using ReactorPtr = Shared<Unique<T>>;

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
    if constexpr(!std::is_same_v<reactor_result_t<T>, void>) {
      pybind11::implicitly_convertible<T, Box<void>>();
    }
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
    auto reactor = pybind11::class_<T, ReactorPtr<T>, Options...>(module,
      name.c_str())
      .def("commit",
        [] (ReactorPtr<T>& self, int sequence) {
          return self.commit(sequence);
        })
      .def("eval",
        [] (ReactorPtr<T>& self) {
          return self.eval();
        });
    if constexpr(std::is_same_v<Type, pybind11::object>) {
      reactor.def("__add__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self + to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__sub__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self - to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__mul__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self * to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__truediv__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self / to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__mod__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self % to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__xor__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self ^ to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__and__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self & to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__or__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self | to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__invert__",
        [] (ReactorPtr<T>& self) {
          return Box(~self);
        }, pybind11::is_operator());
      reactor.def("__lshift__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self << to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__rshift__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self >> to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__lt__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self < to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__le__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self <= to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__ge__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self >= to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__gt__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(self > to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__neg__",
        [] (ReactorPtr<T>& self) {
          return Box(-self);
        }, pybind11::is_operator());
      reactor.def("__pos__",
        [] (ReactorPtr<T>& self) {
          return Box(+self);
        }, pybind11::is_operator());
    }
    if constexpr(!std::is_same_v<T, Box<Type>>) {
      implicitly_convertible_to_box<T>();
    }
    return reactor;
  }
}

#endif

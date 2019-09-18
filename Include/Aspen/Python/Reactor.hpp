#ifndef ASPEN_PYTHON_REACTOR_HPP
#define ASPEN_PYTHON_REACTOR_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Aspen/Box.hpp"
#include "Aspen/Conversions.hpp"
#include "Aspen/Operators.hpp"
#include "Aspen/Traits.hpp"
#include "Aspen/Python/DllExports.hpp"
#include "Aspen/Python/Object.hpp"
#include "Aspen/Python/PythonBox.hpp"
#include "Aspen/Python/ReactorPtr.hpp"
#include "Aspen/Python/Registry.hpp"

namespace Aspen {

  /** Casts a C++ reactor to an Python object. */
  template<typename R>
  decltype(auto) to_object(R&& reactor) {
    if constexpr(std::is_same_v<reactor_result_t<R>, pybind11::object>) {
      return Box(std::forward<R>(reactor));
    } else if constexpr(std::is_same_v<reactor_result_t<R>, void>) {
      return Box(lift(
        [] (const Maybe<void>& result) {
          result.get();
          return pybind11::object(pybind11::none());
        }, std::forward<R>(reactor)));
    } else {
      return Box(ConversionReactor(std::forward<R>(reactor),
        [] (auto&& value) {
          return pybind11::cast(std::forward<decltype(value)>(value));
        }));
    }
  }

  /** Tests if a Python object represents a reactor. */
  ASPEN_EXPORT_DLL bool is_python_reactor(const pybind11::object& value);

  /** Wraps a Python object into an appropriate reactor. */
  template<typename T = pybind11::object>
  Box<T> to_python_reactor(pybind11::object value) {
    auto& boxers = find_boxers(value);
    if(boxers.m_boxer != nullptr) {
      if constexpr(std::is_same_v<T, pybind11::object>) {
        return boxers.m_object_boxer(value);
      } else if constexpr(std::is_same_v<T, void>) {
        return boxers.m_void_boxer(value);
      } else {
        auto reactor = std::optional<Box<T>>();
        boxers.m_boxer(value, &reactor, typeid(Box<T>));
        if(reactor.has_value()) {
          return std::move(*reactor);
        }
      }
    }
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
    register_reactor<T>(reactor);
    if constexpr(std::is_same_v<Type, pybind11::object>) {
      reactor.def("__add__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self + to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__sub__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self - to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__mul__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self * to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__truediv__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self / to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__mod__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self % to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__xor__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self ^ to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__and__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self & to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__or__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self | to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__invert__",
        [] (ReactorPtr<T>& self) {
          return Box(~*self);
        }, pybind11::is_operator());
      reactor.def("__lshift__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self << to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__rshift__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self >> to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__lt__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self < to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__le__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self <= to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__ge__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self >= to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__gt__",
        [] (ReactorPtr<T>& self, const pybind11::object& object) {
          return Box(*self > to_python_reactor(object));
        }, pybind11::is_operator());
      reactor.def("__neg__",
        [] (ReactorPtr<T>& self) {
          return Box(-*self);
        }, pybind11::is_operator());
      reactor.def("__pos__",
        [] (ReactorPtr<T>& self) {
          return Box(+*self);
        }, pybind11::is_operator());
    }
    if constexpr(!std::is_same_v<T, Box<Type>>) {
      implicitly_convertible_to_box<T>();
    }
    return reactor;
  }
}

#endif

#ifndef ASPEN_PYTHON_GROUP_HPP
#define ASPEN_PYTHON_GROUP_HPP
#include <string>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Aspen/Group.hpp"
#include "Aspen/Python/Reactor.hpp"

namespace Aspen {

  /** Exports a Group evaluating to a Python object. */
  void export_group(pybind11::module& module);

  /**
   * Exports the generic Group reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_group(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Group";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<Group<T>>(module, name)
      .def(pybind11::init<T>());
    if constexpr(!std::is_same_v<T, Box<Box<pybind11::object>>>) {
      pybind11::implicitly_convertible<Group<T>,
        Group<Box<Box<pybind11::object>>>>();
    }
  }
}

#endif

#ifndef ASPEN_PYTHON_CONCUR_HPP
#define ASPEN_PYTHON_CONCUR_HPP
#include <string>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Aspen/Concur.hpp"
#include "Aspen/Python/Reactor.hpp"

namespace Aspen {

  /** Exports a Concur evaluating to a Python object. */
  void export_concur(pybind11::module& module);

  /**
   * Exports the generic Concur reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_concur(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Concur";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<Concur<T>>(module, name)
      .def(pybind11::init<T>());
    if constexpr(!std::is_same_v<T, SharedBox<SharedBox<pybind11::object>>>) {
      pybind11::implicitly_convertible<Concur<T>,
        Concur<SharedBox<SharedBox<pybind11::object>>>>();
    }
  }
}

#endif

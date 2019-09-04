#ifndef ASPEN_PYTHON_CHAIN_HPP
#define ASPEN_PYTHON_CHAIN_HPP
#include <string>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Aspen/Chain.hpp"
#include "Aspen/Python/Reactor.hpp"

namespace Aspen {

  /** Exports a Chain evaluating to a Python object. */
  void export_chain(pybind11::module& module);

  /**
   * Exports the generic Chain reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename A, typename B>
  void export_chain(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Chain";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<Chain<A, B>>(module, name)
      .def(pybind11::init<A, B>());
    if constexpr(!std::is_same_v<A, Box<pybind11::object>> ||
        !std::is_same_v<B, Box<pybind11::object>>) {
      pybind11::implicitly_convertible<Chain<A, B>,
        Chain<Box<pybind11::object>, Box<pybind11::object>>>();
    }
  }
}

#endif

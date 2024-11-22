export module Aspen:Chain;

import <string>;
import <type_traits>;
import <pybind11/pybind11.h>;
import :Chain;
#include "Aspen/Python/Reactor.hpp"

export namespace Aspen {

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
    if constexpr(!std::is_same_v<A, SharedBox<pybind11::object>> ||
        !std::is_same_v<B, SharedBox<pybind11::object>>) {
      pybind11::implicitly_convertible<Chain<A, B>,
        Chain<SharedBox<pybind11::object>, SharedBox<pybind11::object>>>();
    }
  }
}

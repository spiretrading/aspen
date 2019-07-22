#ifndef ASPEN_PYTHON_CHAIN_HPP
#define ASPEN_PYTHON_CHAIN_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Aspen/Chain.hpp"
#include "Aspen/Python/Box.hpp"

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
    pybind11::class_<Chain<A, B>>(module, name.c_str())
      .def(pybind11::init<A, B>())
      .def("commit", &Chain<A, B>::commit)
      .def("eval", &Chain<A, B>::eval);
    implicitly_convertible_to_box<Chain<A, B>>();
    if constexpr(!std::is_same_v<A, Box<pybind11::object>> ||
        !std::is_same_v<B, Box<pybind11::object>>) {
      pybind11::implicitly_convertible<Chain<A, B>,
        Chain<Box<pybind11::object>, Box<pybind11::object>>>();
    }
  }
}

#endif

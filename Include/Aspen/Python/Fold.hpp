#ifndef ASPEN_PYTHON_FOLD_HPP
#define ASPEN_PYTHON_FOLD_HPP
#include <string>
#include <type_traits>
#include <pybind11/pybind11.h>
#include "Aspen/Fold.hpp"
#include "Aspen/Python/Reactor.hpp"

namespace Aspen {

  /** Exports a FoldArgument evaluating to a Python object. */
  void export_fold_argument(pybind11::module& module);

  /**
   * Exports the generic FoldArgument reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_fold_argument(pybind11::module& module,
      const std::string& prefix) {
    auto name = prefix + "FoldArgument";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<FoldArgument<T>>(module, name)
      .def(pybind11::init<>());
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<FoldArgument<T>,
        FoldArgument<pybind11::object>>();
    }
  }

  /** Exports a Fold reactor evaluating to a Python object. */
  void export_fold(pybind11::module& module);

  /**
   * Exports the generic Fold reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename E, typename S>
  void export_fold(pybind11::module& module, const std::string& prefix) {
    using Type = reactor_result_t<Fold<E, S>>;
    auto name = prefix + "Fold";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_fold_argument<Type>(module, prefix);
    export_reactor<Fold<E, S>>(module, name)
      .def(pybind11::init<E, Shared<FoldArgument<Type>>,
        Shared<FoldArgument<Type>>, S>());
    if constexpr(!std::is_same_v<E, Box<pybind11::object>> ||
        !std::is_same_v<S, Box<pybind11::object>>) {
      pybind11::implicitly_convertible<Fold<E, S>,
        Fold<Box<pybind11::object>, Box<pybind11::object>>>();
    }
  }
}

#endif

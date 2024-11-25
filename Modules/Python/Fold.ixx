export module Aspen.Python:Fold;

import std;
import <pybind11/pybind11.h>;
import Aspen;
import :Reactor;

export namespace Aspen {

  /**
   * Exports the generic FoldArgument reactor.
   * @param module The module to export to.
   * @param prefix The prefix to use when forming the type name.
   */
  template<typename T>
  void export_fold_argument(
      pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "FoldArgument";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_reactor<FoldArgument<T>>(module, name).
      def(pybind11::init<>());
    if constexpr(!std::is_same_v<T, pybind11::object>) {
      pybind11::implicitly_convertible<
        FoldArgument<T>, FoldArgument<pybind11::object>>();
    }
  }

  /** Exports a FoldArgument evaluating to a Python object. */
  void export_fold_argument(pybind11::module& module) {
    export_fold_argument<pybind11::object>(module, "");
    module.def("make_fold_argument", [] {
      return make_fold_argument<pybind11::object>();
    });
  }

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
    export_reactor<Fold<E, S>>(module, name).
      def(pybind11::init<E, Shared<FoldArgument<Type>>,
        Shared<FoldArgument<Type>>, S>());
    if constexpr(!std::is_same_v<E, SharedBox<pybind11::object>> ||
        !std::is_same_v<S, SharedBox<pybind11::object>>) {
      pybind11::implicitly_convertible<Fold<E, S>,
        Fold<SharedBox<pybind11::object>, SharedBox<pybind11::object>>>();
    }
  }

  /** Exports a Fold reactor evaluating to a Python object. */
  void export_fold(pybind11::module& module) {
    export_fold<SharedBox<pybind11::object>, SharedBox<pybind11::object>>(
      module, "");
    module.def("fold",
      [] (pybind11::object evaluator,
          Shared<FoldArgument<pybind11::object>> left,
          Shared<FoldArgument<pybind11::object>> right,
          pybind11::object series) {
        return shared_box(fold(to_python_reactor(std::move(evaluator)),
          std::move(left), std::move(right),
          to_python_reactor(std::move(series))));
      });
    module.def("fold", [] (pybind11::object f, pybind11::object series) {
      return shared_box(fold(
        [f = std::move(f)] (
            const pybind11::object& a, const pybind11::object& b) {
          return f(a, b);
        }, to_python_reactor(std::move(series))));
    });
  }
}

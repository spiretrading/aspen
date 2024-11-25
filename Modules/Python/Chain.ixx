export module Aspen.Python:Chain;

import std;
import <pybind11/pybind11.h>;
import Aspen;
import :Reactor;

export namespace Aspen {

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
    export_reactor<Chain<A, B>>(module, name).
      def(pybind11::init<A, B>());
    if constexpr(!std::is_same_v<A, SharedBox<pybind11::object>> ||
        !std::is_same_v<B, SharedBox<pybind11::object>>) {
      pybind11::implicitly_convertible<Chain<A, B>,
        Chain<SharedBox<pybind11::object>, SharedBox<pybind11::object>>>();
    }
  }

  /** Exports a Chain evaluating to a Python object. */
  void export_chain(pybind11::module& module) {
    export_chain<SharedBox<pybind11::object>, SharedBox<pybind11::object>>(
      module, "");
    module.def("chain", [] (const pybind11::args& arguments) {
      if(pybind11::len(arguments) == 0) {
        return shared_box(None<pybind11::object>());
      } else if(pybind11::len(arguments) == 1) {
        return to_python_reactor(arguments[0]);
      } else {
        auto size = pybind11::len(arguments);
        auto c = to_python_reactor(arguments[size - 1]);
        for(auto i = size - 1; i-- > 0;) {
          c = shared_box(chain(to_python_reactor(arguments[i]), std::move(c)));
        }
        return c;
      }
    });
  }
}

export module Aspen.Python:Lift;

import <pybind11/pybind11.h>;
import Aspen;
import :Reactor;

struct CallableWrapper {
  pybind11::object operator ()(const pybind11::args& arguments) {
    return m_callable(*arguments);
  }

  pybind11::object m_callable;
};

struct ArgumentsReactor {
  using Type = pybind11::args;
  Aspen::CommitHandler<Aspen::SharedBox<pybind11::object>> m_arguments;

  explicit ArgumentsReactor(const pybind11::args& arguments)
    : m_arguments([&] {
        auto children = std::vector<Aspen::SharedBox<pybind11::object>>();
        for(auto i = std::size_t(0); i != pybind11::len(arguments); ++i) {
          children.emplace_back(Aspen::to_python_reactor(arguments[i]));
        }
        return children;
      }()) {}

  Aspen::State commit(int sequence) noexcept {
    return m_arguments.commit(sequence);
  }

  pybind11::args eval() const {
    auto arguments = pybind11::tuple(m_arguments.size());
    for(auto i = std::size_t(0); i != m_arguments.size(); ++i) {
      arguments[i] = m_arguments.get(i).eval();
    }
    return pybind11::args(std::move(arguments));
  }
};

export namespace Aspen {

  /** Exports the Lift class. */
  void export_lift(pybind11::module& module) {
    using PythonLift = Lift<CallableWrapper, SharedBox<pybind11::args>>;
    export_reactor<PythonLift>(module, "Lift").
      def(pybind11::init(
        [] (pybind11::object callable, const pybind11::args& arguments) {
          return PythonLift(
            CallableWrapper(std::move(callable)), ArgumentsReactor(arguments));
        }));
    module.def("lift",
      [] (pybind11::object callable, const pybind11::args& arguments) {
        return PythonLift(
          CallableWrapper(std::move(callable)), ArgumentsReactor(arguments));
      });
  }
}

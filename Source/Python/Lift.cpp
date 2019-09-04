#include "Aspen/Python/Lift.hpp"
#include "Aspen/CommitHandler.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Python/Box.hpp"
#include "Aspen/Python/Reactor.hpp"

using namespace Aspen;
using namespace pybind11;

namespace {
  struct CallableWrapper {
    object operator ()(const args& arguments) {
      return m_callable(*arguments);
    }

    object m_callable;
  };

  struct ArgumentsReactor {
    using Type = args;
    std::vector<Shared<Box<object>>> m_arguments;
    CommitHandler m_handler;

    explicit ArgumentsReactor(const args& arguments)
      : m_arguments(
          [&] {
            auto children = std::vector<Shared<Box<object>>>();
            for(auto i = std::size_t(0); i != len(arguments); ++i) {
              children.emplace_back(to_python_reactor(arguments[i]));
            }
            return children;
          }()),
        m_handler(
          [&] {
            auto children = std::vector<Box<void>>();
            for(auto& argument : m_arguments) {
              children.emplace_back(argument);
            }
            return children;
          }()) {}

    State commit(int sequence) noexcept {
      return m_handler.commit(sequence);
    }

    args eval() const {
      auto arguments = tuple(m_arguments.size());
      for(auto i = std::size_t(0); i != m_arguments.size(); ++i) {
        arguments[i] = m_arguments[i].eval();
      }
      return args(std::move(arguments));
    }
  };
}

void Aspen::export_lift(pybind11::module& module) {
  using PythonLift = Lift<CallableWrapper, Box<args>>;
  export_reactor<PythonLift>(module, "Lift")
    .def(init(
      [] (object callable, const args& arguments) {
        return PythonLift(CallableWrapper{std::move(callable)},
          ArgumentsReactor(arguments));
      }));
  module.def("lift", [] (object callable, const args& arguments) {
    return PythonLift(CallableWrapper{std::move(callable)},
      ArgumentsReactor(arguments));
  });
}

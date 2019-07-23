#include "Aspen/Python/Lift.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

namespace {
  struct CallableWrapper {
    FunctionEvaluation<object> operator ()(const args& arguments) {
      return m_callable(*arguments);
    }

    object m_callable;
  };

  struct ArgumentsReactor {
    using Type = args;
    CommitHandler m_handler;
    std::vector<PythonBox<object>> m_arguments;

    explicit ArgumentsReactor(const args& arguments)
      : m_handler(
          [&] {
            auto children = std::vector<Box<void>>();
            for(auto i = std::size_t(0); i != len(arguments); ++i) {
              children.emplace_back(PythonBox<object>(arguments[i]));
            }
            return children;
          }()) {
      for(auto i = std::size_t(0); i != len(arguments); ++i) {
        m_arguments.emplace_back(arguments[i]);
      }
    }

    State commit(int sequence) {
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
  class_<PythonLift>(module, "Lift")
    .def(init(
      [] (object callable, const args& arguments) {
        return PythonLift(CallableWrapper{std::move(callable)},
          ArgumentsReactor(arguments));
      }))
    .def("commit", &PythonLift::commit)
    .def("eval", &PythonLift::eval);
  implicitly_convertible_to_box<PythonLift>();
  module.def("lift", [] (object callable, const args& arguments) {
    return PythonLift(CallableWrapper{std::move(callable)},
      ArgumentsReactor(arguments));
  });
}

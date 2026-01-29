#include "Aspen/Python/Executor.hpp"
#include "Aspen/Executor.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Python/GilAcquireReactor.hpp"

using namespace Aspen;
using namespace pybind11;

namespace {
  struct ExecutorReactor {
    using Type = void;
    SharedBox<void> m_reactor;

    ExecutorReactor(SharedBox<void> reactor)
      : m_reactor(std::move(reactor)) {}

    State commit(int sequence) noexcept {
      auto state = m_reactor.commit(sequence);
      if(has_evaluation(state)) {
        try {
          m_reactor.eval();
        } catch(error_already_set& e) {
          e.restore();
          PyErr_Print();
        } catch(const std::exception& e) {
          PySys_WriteStderr("Exception: %s\n", e.what());
        } catch(...) {
          PySys_WriteStderr("Unknown exception occurred\n");
        }
      }
      return state;
    }

    void eval() const {
      m_reactor.eval();
    }
  };
}

void Aspen::export_executor(pybind11::module& module) {
  class_<Executor>(module, "Executor")
    .def(init(
      [] (SharedBox<void> reactor) {
        return std::make_unique<Executor>(
          GilAcquireReactor(ExecutorReactor(std::move(reactor))));
      }))
    .def("run_until_none", &Executor::run_until_none)
    .def("run_until_complete", &Executor::run_until_complete,
      call_guard<gil_scoped_release>());
}

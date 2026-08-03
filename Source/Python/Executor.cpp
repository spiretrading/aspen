#include "Aspen/Python/Executor.hpp"
#include <atomic>
#include <memory>
#include <utility>
#include "Aspen/Executor.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Python/GilAcquireReactor.hpp"

using namespace Aspen;
using namespace pybind11;

namespace {
  struct ExecutorReactor {
    using Type = void;
    SharedBox<void> m_reactor;
    std::shared_ptr<std::atomic_bool> m_is_complete;

    ExecutorReactor(SharedBox<void> reactor,
      std::shared_ptr<std::atomic_bool> is_complete)
      : m_reactor(std::move(reactor)),
        m_is_complete(std::move(is_complete)) {}

    State commit(std::uint64_t sequence) noexcept {
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
      if(is_complete(state)) {
        m_is_complete->store(true);
      }
      return state;
    }

    void eval() const {
      m_reactor.eval();
    }
  };

  class PythonExecutor {
    public:
      explicit PythonExecutor(SharedBox<void> reactor)
        : m_is_complete(std::make_shared<std::atomic_bool>(false)),
          m_is_aborted(std::make_shared<std::atomic_bool>(false)),
          m_executor(GilAcquireReactor(
            ExecutorReactor(std::move(reactor), m_is_complete))) {}

      void run_until_none() {
        m_executor.run_until_none();
      }

      void run_until_complete() {
        {
          auto release = gil_scoped_release();
          m_executor.run_until_complete();
        }
        if(!m_is_complete->load() && !m_is_aborted->load()) {
          PyErr_SetInterrupt();
          if(PyErr_CheckSignals() != 0) {
            throw error_already_set();
          }
        }
      }

      void abort() {
        m_is_aborted->store(true);
        auto release = gil_scoped_release();
        m_executor.abort();
      }

    private:
      std::shared_ptr<std::atomic_bool> m_is_complete;
      std::shared_ptr<std::atomic_bool> m_is_aborted;
      Executor m_executor;
  };
}

void Aspen::export_executor(pybind11::module& module) {
  class_<PythonExecutor>(module, "Executor")
    .def(init<SharedBox<void>>())
    .def("run_until_none", &PythonExecutor::run_until_none)
    .def("run_until_complete", &PythonExecutor::run_until_complete)
    .def("abort", &PythonExecutor::abort);
}

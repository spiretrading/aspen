#include "Aspen/Python/Executor.hpp"
#include "Aspen/Executor.hpp"
#include "Aspen/Python/GilAcquireReactor.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_executor(pybind11::module& module) {
  class_<Executor>(module, "Executor")
    .def(init(
      [] (SharedBox<void> reactor) {
        return std::make_unique<Executor>(
          GilAcquireReactor(std::move(reactor)));
      }))
    .def("run_until_none", &Executor::run_until_none)
    .def("run_until_complete", &Executor::run_until_complete,
      call_guard<gil_scoped_release>());
}

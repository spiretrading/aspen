#include "Aspen/Python/StateReactor.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_state_reactor(pybind11::module& module) {
  export_box<State>(module, "State");
  export_state_reactor<SharedBox<object>>(module, "");
}

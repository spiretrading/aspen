#include <pybind11/pybind11.h>
#include "Aspen/Python/Aspen.hpp"

using namespace Aspen;
using namespace pybind11;

PYBIND11_MODULE(aspen, module) {
  export_box(module);
  export_chain(module);
  export_commit_handler(module);
  export_constant(module);
  export_executor(module);
  export_first(module);
  export_fold(module);
  export_last(module);
  export_lift(module);
  export_none(module);
  export_perpetual(module);
  export_queue(module);
  export_range(module);
  export_state(module);
  export_switch(module);
  export_trigger(module);
}

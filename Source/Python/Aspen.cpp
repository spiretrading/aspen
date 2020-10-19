#include "Aspen/Python/Aspen.hpp"
#include <pybind11/pybind11.h>

using namespace Aspen;
using namespace pybind11;

template struct Aspen::Details::StaticTrigger<void>;

PYBIND11_MODULE(aspen, module) {
  export_box(module);
  export_cell(module);
  export_chain(module);
  export_commit_handler(module);
  export_concat(module);
  export_concur(module);
  export_constant(module);
  export_count(module);
  export_discard(module);
  export_executor(module);
  export_first(module);
  export_fold(module);
  export_group(module);
  export_last(module);
  export_lift(module);
  export_none(module);
  export_override(module);
  export_perpetual(module);
  export_proxy(module);
  export_queue(module);
  export_range(module);
  export_state(module);
  export_state_reactor(module);
  export_switch(module);
  export_trigger(module);
  export_unconsecutive(module);
  export_until(module);
  export_when(module);
}

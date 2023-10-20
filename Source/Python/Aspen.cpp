#include "Aspen/Python/Aspen.hpp"
#include <pybind11/pybind11.h>

using namespace Aspen;
using namespace pybind11;

template struct Aspen::Details::StaticTrigger<void>;

PYBIND11_MODULE(aspen, m) {
  export_box(m);
  export_cell(m);
  export_chain(m);
  export_commit_handler(m);
  export_concat(m);
  export_concur(m);
  export_constant(m);
  export_count(m);
  export_discard(m);
  export_distinct(m);
  export_executor(m);
  export_first(m);
  export_fold(m);
  export_group(m);
  export_last(m);
  export_lift(m);
  export_none(m);
  export_override(m);
  export_perpetual(m);
  export_previous(m);
  export_proxy(m);
  export_queue(m);
  export_range(m);
  export_state(m);
  export_state_reactor(m);
  export_switch(m);
  export_trigger(m);
  export_unconsecutive(m);
  export_until(m);
  export_when(m);
}

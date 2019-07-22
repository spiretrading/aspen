#include <pybind11/pybind11.h>
#include "Aspen/Python/Box.hpp"
#include "Aspen/Python/Chain.hpp"
#include "Aspen/Python/CommitHandler.hpp"
#include "Aspen/Python/Constant.hpp"
#include "Aspen/Python/First.hpp"
#include "Aspen/Python/Last.hpp"
#include "Aspen/Python/None.hpp"
#include "Aspen/Python/Perpetual.hpp"
#include "Aspen/Python/Queue.hpp"
#include "Aspen/Python/Range.hpp"
#include "Aspen/Python/State.hpp"
#include "Aspen/Python/Trigger.hpp"

using namespace Aspen;
using namespace pybind11;

PYBIND11_MODULE(aspen, module) {
  export_box(module);
  export_chain(module);
  export_commit_handler(module);
  export_constant(module);
  export_first(module);
  export_last(module);
  export_none(module);
  export_perpetual(module);
  export_queue(module);
  export_range(module);
  export_state(module);
  export_trigger(module);
}

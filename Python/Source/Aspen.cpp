#include <pybind11/pybind11.h>
#include "Aspen/Python/Box.hpp"
#include "Aspen/Python/Chain.hpp"
#include "Aspen/Python/CommitHandler.hpp"
#include "Aspen/Python/Constant.hpp"
#include "Aspen/Python/First.hpp"
#include "Aspen/Python/Last.hpp"
#include "Aspen/Python/Range.hpp"
#include "Aspen/Python/State.hpp"

using namespace Aspen;
using namespace pybind11;

PYBIND11_MODULE(aspen, module) {
  export_box(module);
  export_chain(module);
  export_commit_handler(module);
  export_constant(module);
  export_first(module);
  export_last(module);
  export_range(module);
  export_state(module);
}

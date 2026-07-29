#include "Aspen/Python/Count.hpp"
#include <cstdint>
#include "Aspen/Count.hpp"
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_count(pybind11::module& module) {
  export_box<std::uint64_t>(module, "UInt64");
  module.def("count",
    [] (SharedBox<void> series) {
      return shared_box(count(std::move(series)));
    });
}

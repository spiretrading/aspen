#include "Aspen/Python/Group.hpp"
#include "Aspen/Python/None.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_group(pybind11::module& module) {
  export_group<SharedBox<object>, SharedBox<object>>(module, "");
  module.def("group",
    [] (const args& arguments) {
      if(len(arguments) == 0) {
        return shared_box(None<object>());
      } else if(len(arguments) == 1) {
        return to_python_reactor(arguments[0]);
      } else {
        auto size = len(arguments);
        auto c = to_python_reactor(arguments[size - 1]);
        for(auto i = size - 1; i-- > 0;) {
          c = shared_box(group(to_python_reactor(arguments[i]), std::move(c)));
        }
        return c;
      }
    });
}

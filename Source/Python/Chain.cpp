#include "Aspen/Python/Chain.hpp"
#include "Aspen/Python/None.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_chain(pybind11::module& module) {
  export_chain<SharedBox<object>, SharedBox<object>>(module, "");
  module.def("chain",
    [] (const args& arguments) {
      if(len(arguments) == 0) {
        return SharedBox(None<object>());
      } else if(len(arguments) == 1) {
        return to_python_reactor(arguments[0]);
      } else {
        auto size = len(arguments);
        auto c = to_python_reactor(arguments[size - 1]);
        for(auto i = size - 1; i-- > 0;) {
          c = SharedBox(chain(to_python_reactor(arguments[i]), std::move(c)));
        }
        return c;
      }
    });
}

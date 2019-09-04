#include "Aspen/Python/Fold.hpp"
#include "Aspen/Fold.hpp"

using namespace Aspen;
using namespace pybind11;

void Aspen::export_fold_argument(pybind11::module& module) {
  export_fold_argument<object>(module, "");
  module.def("make_fold_argument",
    [] {
      return make_fold_argument<object>();
    });
}

void Aspen::export_fold(pybind11::module& module) {
  export_fold<Box<object>, Box<object>>(module, "");
  module.def("fold",
    [] (object evaluator, Shared<FoldArgument<object>> left,
        Shared<FoldArgument<object>> right, object series) {
      return Box(fold(to_python_reactor(std::move(evaluator)), std::move(left),
        std::move(right), to_python_reactor(std::move(series))));
    });
  module.def("fold",
    [] (object f, object series) {
      return Box(fold(
        [f = std::move(f)] (const object& a, const object& b) {
          return f(a, b);
        }, to_python_reactor(std::move(series))));
    });
}

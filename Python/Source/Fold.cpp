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
    [] (const Box<object>& evaluator,
        std::shared_ptr<FoldArgument<object>> left,
        std::shared_ptr<FoldArgument<object>> right,
        const Box<object>& series) {
      return Box(fold(evaluator, std::move(left), std::move(right), series));
    });
  module.def("fold",
    [] (const object& f, const Box<object>& series) {
      return Box(
        fold([=] (const object& a, const object& b) { return f(a, b); },
        series));
    });
}

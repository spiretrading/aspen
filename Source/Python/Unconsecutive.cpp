#include "Aspen/Python/Unconsecutive.hpp"
#include <optional>
#include "Aspen/Python/Box.hpp"

using namespace Aspen;
using namespace pybind11;

namespace {
  struct UnconsecutiveCore {
    std::optional<object> m_previous;

    std::optional<object> operator ()(Maybe<object>&& series) {
      return (*this)(std::move(*series));
    }

    std::optional<object> operator ()(object series) {
      if(m_previous.has_value() && m_previous->equal(series)) {
        return std::nullopt;
      } else {
        m_previous.emplace(std::move(series));
        return *m_previous;
      }
    }
  };
}

void Aspen::export_unconsecutive(pybind11::module& module) {
  module.def("unconsecutive",
    [] (SharedBox<object> series) {
      return shared_box(Lift(UnconsecutiveCore(), std::move(series)));
    });
}

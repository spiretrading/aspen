export module Aspen.Python:Unconsecutive;

import std;
import <pybind11/pybind11.h>;
import Aspen;

struct UnconsecutiveCore {
  std::optional<pybind11::object> m_previous;

  std::optional<pybind11::object> operator ()(
      Aspen::Maybe<pybind11::object>&& series) {
    return (*this)(std::move(*series));
  }

  std::optional<pybind11::object> operator ()(pybind11::object series) {
    if(m_previous && m_previous->equal(series)) {
      return std::nullopt;
    } else {
      m_previous.emplace(std::move(series));
      return *m_previous;
    }
  }
};

export namespace Aspen {

  /** Exports the unconsecutive reactor to Python. */
  void export_unconsecutive(pybind11::module& module) {
    module.def("unconsecutive", [] (SharedBox<pybind11::object> series) {
      return shared_box(Lift(UnconsecutiveCore(), std::move(series)));
    });
  }
}

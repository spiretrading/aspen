#ifndef ASPEN_UNCONSECUTIVE_HPP
#define ASPEN_UNCONSECUTIVE_HPP
#include <optional>
#include <utility>
#include "Aspen/Lift.hpp"

namespace Aspen {
namespace Details {
  template<typename Type>
  struct UnconsecutiveCore {
    std::optional<Type> m_previous;

    template<typename T>
    std::optional<Type> operator ()(T&& series) {
      if(m_previous == series) {
        return std::nullopt;
      } else {
        m_previous.emplace(std::forward<T>(series));
        return *m_previous;
      }
    }
  };
}

  /**
   * Implements a reactor that only evaluates when unconsecutive values are
   * produced.
   * @param series The series producing the values.
   */
  template<typename Series>
  auto unconsecutive(Series&& series) {
    using Type = reactor_result_t<Series>;
    return Lift(Details::UnconsecutiveCore<Type>(),
      std::forward<Series>(series));
  }
}

#endif

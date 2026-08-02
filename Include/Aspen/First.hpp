#ifndef ASPEN_FIRST_HPP
#define ASPEN_FIRST_HPP
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to the first value it receives from its
   * source.
   * @param source The source that will provide the value to evaluate to.
   * @return A reactor evaluating to the first value of the <i>source</i>.
   */
  template<typename Source> requires IsReactor<to_reactor_t<Source>>
  auto first(Source&& source) {
    return lift([] (const auto& value) noexcept {
      return FunctionEvaluation(value, State::COMPLETE_EVALUATED);
    }, std::forward<Source>(source));
  }
}

#endif

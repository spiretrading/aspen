#ifndef ASPEN_LAST_HPP
#define ASPEN_LAST_HPP
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/State.hpp"
#include "Aspen/StateReactor.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to the last value it receives from its
   * source.
   * @param source The source that will provide the value to evaluate to.
   * @return A reactor evaluating to the last value of the <i>source</i>.
   */
  template<typename Source> requires IsReactor<to_reactor_t<Source>>
  auto last(Source&& source) {
    using Type = reactor_result_t<Source>;
    auto source_reactor = Shared(std::forward<Source>(source));
    return lift([] (const auto& value, State state) noexcept ->
        FunctionEvaluation<Type> {
      if(is_complete(state)) {
        return FunctionEvaluation(value, State::COMPLETE_EVALUATED);
      }
      return State::NONE;
    }, source_reactor, StateReactor(source_reactor));
  }
}

#endif

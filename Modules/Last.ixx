export module Aspen:Last;

import <utility>;
import :Lift;
import :Shared;
import :StateReactor;
import :Traits;

export namespace Aspen {

  /**
   * Implements a reactor that evaluates to the last value it receives from its
   * source.
   * @param source The source that will provide the value to evaluate to.
   */
  template<typename Reactor>
  auto last(Reactor&& source) {
    using Type = reactor_result_t<Reactor>;
    auto source_reactor = Shared(std::forward<Reactor>(source));
    return Lift(
      [] (auto&& value, State state) noexcept -> FunctionEvaluation<Type> {
        if(is_complete(state)) {
          return FunctionEvaluation<Type>(std::forward<decltype(value)>(value),
            State::COMPLETE_EVALUATED);
        }
        return State::NONE;
      }, source_reactor, StateReactor(source_reactor));
  }
}

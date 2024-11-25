export module Aspen:Previous;

import std;
import :Lift;
import :Shared;
import :StateReactor;
import :Traits;

export namespace Aspen {

  /**
   * Implements a reactor that evaluates to its previous value.
   * @param source The source to evaluate.
   */
  template<typename Reactor>
  auto previous(Reactor&& source) {
    using Type = reactor_result_t<Reactor>;
    auto source_reactor = Shared(std::forward<Reactor>(source));
    return Lift([previous = std::optional<Type>()]
        (auto&& value, State state) mutable noexcept ->
          FunctionEvaluation<Type> {
      if(is_complete(state)) {
        if(!previous) {
          if(has_evaluation(state)) {
            previous.emplace(std::forward<decltype(value)>(value));
            return State::CONTINUE;
          }
          return State::NONE;
        } else {
          return std::move(*previous);
        }
      }
      if(!previous) {
        previous.emplace(std::forward<decltype(value)>(value));
        return State::NONE;
      }
      auto result = std::move(*previous);
      *previous = std::forward<decltype(value)>(value);
      return result;
    }, source_reactor, StateReactor(source_reactor));
  }
}

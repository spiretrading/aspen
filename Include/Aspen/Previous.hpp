#ifndef ASPEN_PREVIOUS_HPP
#define ASPEN_PREVIOUS_HPP
#include <optional>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/StateReactor.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

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

#endif

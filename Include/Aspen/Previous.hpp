#ifndef ASPEN_PREVIOUS_HPP
#define ASPEN_PREVIOUS_HPP
#include <optional>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/State.hpp"
#include "Aspen/StateReactor.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to its previous value.
   * @param source The source to evaluate.
   * @return A reactor evaluating to the <i>source</i>'s previous value.
   */
  template<typename Source> requires IsReactor<to_reactor_t<Source>>
  auto previous(Source&& source) {
    using Type = reactor_result_t<Source>;
    using Series = to_reactor_t<Source>;
    auto source_reactor = Shared(std::forward<Source>(source));
    return lift([previous = std::optional<Type>(), is_absorbed = false] (
        const auto& value, State state) mutable noexcept ->
          FunctionEvaluation<std::remove_cvref_t<decltype(value)>> {
      if constexpr(!is_noexcept_reactor_v<Series>) {
        if(has_evaluation(state) && value.has_exception()) {
          return Maybe<Type>(value.get_exception());
        }
      }
      if(is_complete(state)) {
        if(has_evaluation(state) && !is_absorbed) {
          is_absorbed = true;
          if(!previous) {
            previous.emplace(value);
            return State::CONTINUE;
          }
          auto result = std::move(*previous);
          *previous = value;
          return FunctionEvaluation<std::remove_cvref_t<decltype(value)>>(
            std::move(result), State::CONTINUE);
        }
        if(!previous) {
          return State::NONE;
        }
        return std::move(*previous);
      }
      if(!has_evaluation(state)) {
        return State::NONE;
      }
      if(!previous) {
        previous.emplace(value);
        return State::NONE;
      }
      auto result = std::move(*previous);
      *previous = value;
      return result;
    }, source_reactor, StateReactor(source_reactor));
  }
}

#endif

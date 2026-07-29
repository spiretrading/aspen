#ifndef ASPEN_PREVIOUS_HPP
#define ASPEN_PREVIOUS_HPP
#include <optional>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Shared.hpp"
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
    return lift([previous = std::optional<Type>()] (const auto& value,
        State state) mutable noexcept -> FunctionEvaluation<Type> {
      if constexpr(!is_noexcept_reactor_v<Series>) {
        if(value.has_exception()) {
          return Maybe<Type>(value.get_exception());
        }
      }
      if(is_complete(state)) {
        if(!previous) {
          if(has_evaluation(state)) {
            previous.emplace(value);
            return State::CONTINUE;
          }
          return State::NONE;
        }
        return std::move(*previous);
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

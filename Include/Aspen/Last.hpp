#ifndef ASPEN_LAST_HPP
#define ASPEN_LAST_HPP
#include <optional>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/StateReactor.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to the last value it receives from its
   * source.
   * @param source The source that will provide the value to evaluate to.
   */
  template<typename Reactor>
  auto last(Reactor&& source) {
    using Type = reactor_result_t<Reactor>;
    auto source_reactor = make_ptr(std::forward<Reactor>(source));
    return Lift(
      [] (auto&& value, State state) noexcept -> FunctionEvaluation<Type> {
        if(is_complete(state)) {
          return FunctionEvaluation<Type>(std::forward<decltype(value)>(value),
            State::COMPLETE_EVALUATED);
        }
        return State::NONE;
      }, std::move(source_reactor), StateReactor(&*source_reactor));
  }
}

#endif

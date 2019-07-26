#ifndef ASPEN_LAST_HPP
#define ASPEN_LAST_HPP
#include <memory>
#include <optional>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/StateReactor.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {
namespace Details {
  template<typename T>
  struct LastCore {
    using Type = T;

    std::optional<Type> operator ()(Type value, State state) noexcept {
      if(is_complete(state)) {
        return value;
      }
      return std::nullopt;
    }
  };
}

  /**
   * Implements a reactor that evaluates to the last value it receives from its
   * source.
   * @param source The source that will provide the value to evaluate to.
   */
  template<typename Reactor>
  auto last(Reactor&& source) {
    auto source_reactor = make_ptr(std::forward<Reactor>(source));
    return Lift(Details::LastCore<reactor_result_t<Reactor>>(),
      std::move(source_reactor), StateReactor(&*source_reactor));
  }
}

#endif

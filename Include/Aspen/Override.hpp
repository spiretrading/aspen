#ifndef ASPEN_OVERRIDE_HPP
#define ASPEN_OVERRIDE_HPP
#include <cstdint>
#include <type_traits>
#include <utility>
#include "Aspen/Concat.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Count.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Operators.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Traits.hpp"
#include "Aspen/Until.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to the values of the reactors produced
   * by its child, where each successively produced reactor overrides the
   * previous.
   * @param producer The reactor producing the reactors to evaluate to.
   * @return A reactor evaluating to the values of the most recently produced
   *         reactor.
   */
  template<typename T> requires
    IsReactor<std::remove_cvref_t<T>> && IsReactor<reactor_result_t<T>>
  auto override(T&& producer) {
    using Reactor = reactor_result_t<T>;
    auto producer_handle = Shared(std::forward<T>(producer));
    auto counter = Shared(count(producer_handle));
    return concat(lift([counter] (const Reactor& reactor, std::uint64_t count) {
      return Shared(until(counter != constant(count), reactor));
    }, std::move(producer_handle), std::move(counter)));
  }
}

#endif

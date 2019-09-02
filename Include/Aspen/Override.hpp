#ifndef ASPEN_OVERRIDE_HPP
#define ASPEN_OVERRIDE_HPP
#include <utility>
#include "Aspen/Concat.hpp"
#include "Aspen/Count.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Operators.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Traits.hpp"
#include "Aspen/Until.hpp"

namespace Aspen {
  template<typename T>
  auto override(T&& producer) {
    using Reactor = reactor_result_t<T>;
    auto producer_handle = Shared(std::forward<T>(producer));
    auto counter = Shared(count(producer_handle));
    return concat(lift(
      [=] (const Reactor& reactor, int count) mutable {
        return Shared(until(counter != constant(count), reactor));
      }, std::move(producer_handle), std::move(counter)));
  }
}

#endif

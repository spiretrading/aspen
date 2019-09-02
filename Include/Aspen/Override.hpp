#ifndef ASPEN_OVERRIDE_HPP
#define ASPEN_OVERRIDE_HPP
#include <utility>
#include "Aspen/Concat.hpp"
#include "Aspen/Count.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Operators.hpp"
#include "Aspen/Traits.hpp"
#include "Aspen/Until.hpp"

namespace Aspen {
  template<typename T>
  auto override(T&& producer) {
    using Reactor = reactor_result_t<T>;
    auto producer_handle = make_ptr(std::forward<T>(producer));
    auto counter = make_ptr(count(&*producer_handle));
    return concat(lift(
      [counter = &*counter] (const Reactor& reactor, int c) mutable {
        return std::shared_ptr(make_until(counter != constant(c), reactor));
      }, std::move(producer_handle), std::move(counter)));
  }
}

#endif

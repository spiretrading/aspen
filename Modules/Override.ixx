export module Aspen:Override;

import std;
import :Concat;
import :Count;
import :Lift;
import :Operators;
import :Shared;
import :Traits;
import :Until;

export namespace Aspen {

  /**
   * Implements a reactor that evaluates to the reactors produced by its child,
   * where each successively produced reactor overrides the previous.
   */
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

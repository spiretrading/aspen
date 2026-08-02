#ifndef ASPEN_REACTOR_HPP
#define ASPEN_REACTOR_HPP
#include <concepts>
#include <cstdint>
#include <type_traits>
#include <utility>
#include "Aspen/State.hpp"

namespace Aspen {

  /**
   * Concept for a reactor, a type that evaluates to a series of values.
   * @param <R> The type to test.
   */
  template<typename R>
  concept IsReactor = requires(R& reactor) {
    typename R::Type;
    { reactor.commit(std::uint64_t()) } noexcept -> std::same_as<State>;
    requires std::same_as<
      std::remove_cvref_t<decltype(std::as_const(reactor).eval())>,
      typename R::Type>;
  };

  /**
   * Concept for a reactor that evaluates to a given type.
   * @param <R> The type to test.
   * @param <T> The type the reactor evaluates to.
   */
  template<typename R, typename T>
  concept IsReactorOf = IsReactor<R> && std::same_as<typename R::Type, T>;
}

#endif

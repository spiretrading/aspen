#ifndef ASPEN_TRAITS_HPP
#define ASPEN_TRAITS_HPP
#include <concepts>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include "Aspen/Constant.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/Reactor.hpp"

namespace Aspen {

  /** Trait used to wrap a type into a reactor. */
  template<typename T>
  struct to_reactor {
    using type = Constant<std::decay_t<T>>;
  };

  template<typename T> requires IsReactor<std::decay_t<T>>
  struct to_reactor<T> {
    using type = std::decay_t<T>;
  };

  template<typename T>
  using to_reactor_t = typename to_reactor<T>::type;

  /** Trait used to determine what type a reactor evaluates to. */
  template<typename T>
  struct reactor_result {
    using type = typename to_reactor_t<std::decay_t<T>>::Type;
  };

  template<typename T>
  using reactor_result_t = typename reactor_result<T>::type;

  template<typename T>
  struct eval_result {
    using type = const T&;
  };

  template<>
  struct eval_result<void> {
    using type = void;
  };

  template<typename T>
  using eval_result_t = typename eval_result<T>::type;

  /** Tests if a reactor's eval method is noexcept. */
  template<typename R> requires IsReactor<std::remove_cvref_t<R>>
  constexpr auto is_noexcept_reactor_v = noexcept(std::declval<R>().eval());

  /**
   * Applies a function to every element of a tuple.
   * @param tuple The tuple containing the elements to apply the function to.
   * @param f The function to apply to each element.
   */
  template<typename... T, typename F> requires (std::invocable<F, T&> && ...)
  void for_each(std::tuple<T...>& tuple, F f) {
    std::apply([&] (T&... elements) {
      (f(elements), ...);
    }, tuple);
  }

  /**
   * Assigns a reactor's evaluation to a value, capturing any exception thrown.
   * @param value The value to assign to.
   * @param reactor The reactor to evaluate.
   */
  template<IsReactor R>
  void try_assign(auto& value, const R& reactor) noexcept {
    if constexpr(is_noexcept_reactor_v<R>) {
      if constexpr(std::is_same_v<reactor_result_t<R>, void>) {
        reactor.eval();
        value = Maybe<void>();
      } else {
        value = reactor.eval();
      }
    } else {
      value = try_call([&] { return reactor.eval(); });
    }
  }
}

#endif

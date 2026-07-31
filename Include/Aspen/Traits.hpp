#ifndef ASPEN_TRAITS_HPP
#define ASPEN_TRAITS_HPP
#include <concepts>
#include <exception>
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

  /** Trait used to determine how a reactor's evaluation is returned. */
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

  /** Trait used to determine the type returned by a reactor's evaluation. */
  template<typename R> requires IsReactor<std::remove_cvref_t<R>>
  using reactor_evaluation_t = decltype(std::declval<const R&>().eval());

  /** Tests if a reactor's evaluation is returned by reference. */
  template<typename R> requires IsReactor<std::remove_cvref_t<R>>
  constexpr auto is_reference_evaluation_v =
    std::is_reference_v<reactor_evaluation_t<R>>;

  /** Trait used to determine the type a series of reactors evaluates to. */
  template<IsReactor... R>
  using common_result_t = std::common_type_t<reactor_result_t<R>...>;

  /**
   * Trait used to determine the type returned by evaluating any one of a series
   * of reactors.
   */
  template<IsReactor... R>
  using common_evaluation_t =
    std::conditional_t<(is_reference_evaluation_v<R> && ...),
      eval_result_t<common_result_t<R...>>, common_result_t<R...>>;

  /** Tests if the eval method of each of a series of reactors is noexcept. */
  template<typename... R> requires (IsReactor<std::remove_cvref_t<R>> && ...)
  constexpr auto is_noexcept_reactor_v =
    (noexcept(std::declval<R>().eval()) && ...);

  /** Tests if evaluating any one of a series of reactors is noexcept. */
  template<IsReactor... R>
  constexpr auto is_noexcept_evaluation_v = is_noexcept_reactor_v<R...> &&
    (std::is_reference_v<common_evaluation_t<R...>> ||
      std::is_void_v<common_result_t<R...>> ||
      std::is_nothrow_copy_constructible_v<common_result_t<R...>>);

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
    } else if constexpr(std::is_same_v<reactor_result_t<R>, void>) {
      value = try_call([&] { return reactor.eval(); });
    } else {
      try {
        value = reactor.eval();
      } catch(...) {
        value = std::current_exception();
      }
    }
  }
}

#endif

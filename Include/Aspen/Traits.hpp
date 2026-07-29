#ifndef ASPEN_TRAITS_HPP
#define ASPEN_TRAITS_HPP
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include "Aspen/Constant.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"

namespace Aspen {

  /** Trait testing whether a type is a reactor. */
  template<typename T, typename=void>
  struct is_reactor : std::false_type {};

  template<typename T>
  struct is_reactor<T, std::enable_if_t<std::is_same_v<
    decltype(std::declval<T>().commit(std::declval<std::uint64_t>())),
    State>>> :
    std::true_type {};

  template<typename T>
  constexpr bool is_reactor_v = is_reactor<T>::value;

  /** Trait used to wrap a type into a reactor. */
  template<typename T, typename=void>
  struct to_reactor {
    using type = Constant<std::decay_t<T>>;
  };

  template<typename T>
  struct to_reactor<T, std::enable_if_t<is_reactor_v<std::decay_t<T>>>> {
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

  /** Tests if a function invoked with a given list of arguments is noexcept. */
  template<typename F, typename... A>
  struct is_noexcept_function : std::bool_constant<
    noexcept(std::declval<F>()(std::declval<A>()...))> {};

  template<typename F, typename... A>
  constexpr auto is_noexcept_function_v = is_noexcept_function<F, A...>::value;

  /** Tests if a reactor's eval method is noexcept. */
  template<typename R>
  struct is_noexcept_reactor : std::bool_constant<
    noexcept(std::declval<R>().eval())> {};

  template<typename R>
  constexpr auto is_noexcept_reactor_v = is_noexcept_reactor<R>::value;

  template<typename... T, typename F>
  void for_each(std::tuple<T...>& t, F&& f) {
    std::apply([&] (T&... elements) {
      (f(elements), ...);
    }, t);
  }

  template<std::size_t I, std::size_t J, typename F>
  void for_each(F&& f) {
    [&] <std::size_t... O> (std::index_sequence<O...>) {
      (f(std::integral_constant<std::size_t, I + O>()), ...);
    }(std::make_index_sequence<J - I>());
  }

  /**
   * Assigns a reactor's evaluation to a value, capturing any exception thrown.
   * @param value The value to assign to.
   * @param reactor The reactor to evaluate.
   */
  template<typename V, typename R>
  void try_assign(V& value, const R& reactor) noexcept {
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

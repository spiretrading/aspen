#ifndef ASPEN_TRAITS_HPP
#define ASPEN_TRAITS_HPP
#include <memory>
#include <type_traits>
#include "Aspen/State.hpp"

namespace Aspen {

  /** Trait testing whether a type is a reactor. */
  template<typename T, typename=void>
  struct is_reactor : std::false_type {};

  template<typename T>
  struct is_reactor<T, std::enable_if_t<
    std::is_same_v<decltype(std::declval<T>().commit(int())), State>>> :
    std::true_type {};

  template<typename T>
  constexpr auto is_reactor_v = is_reactor<T>::value;

  /** Trait testing whether a type is a pointer reactor. */
  template<typename T, typename=void>
  struct is_reactor_pointer : std::false_type {};

  template<typename T>
  struct is_reactor_pointer<T, std::enable_if_t<is_reactor_v<
    decltype(*std::declval<T>())>>> : std::true_type {};

  template<typename T>
  constexpr auto is_reactor_pointer_v = is_reactor_pointer<T>::value;

  /** Trait used to dereference a pointer to a reactor if needed. */
  template<typename T, typename=void>
  struct dereference_reactor {
    using type = std::decay_t<T>;
  };

  template<typename T>
  struct dereference_reactor<T, std::enable_if_t<is_reactor_pointer_v<T>>> :
    dereference_reactor<decltype(*std::declval<T>())> {};

  template<typename T>
  using dereference_reactor_t = typename dereference_reactor<T>::type;

  /** Trait used to determine what type a reactor evaluates to. */
  template<typename T>
  struct reactor_result {
    using type = typename dereference_reactor_t<T>::Type;
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

  template<typename T>
  auto make_ptr(T&& value) {
    if constexpr(is_reactor_pointer_v<std::decay_t<T>>) {
      return std::forward<T>(value);
    } else {
      return std::make_unique<std::decay_t<T>>(std::forward<T>(value));
    }
  }

  template<std::size_t I = 0, typename... T, typename F>
  void for_each(std::tuple<T...>& t, F&& f) {
    if constexpr(I != sizeof...(T)) {
      f(std::get<I>(t));
      for_each<I + 1>(t, std::forward<F>(f));
    }
  }

  template<std::size_t I, std::size_t J, typename F>
  void for_each(F&& f) {
    if constexpr(I != J) {
      f(std::integral_constant<std::size_t, I>());
      for_each<I + 1, J>(std::forward<F>(f));
    }
  }
}

#endif

#ifndef ASPEN_TRAITS_HPP
#define ASPEN_TRAITS_HPP
#include <memory>
#include <type_traits>
#include "Aspen/Constant.hpp"
#include "Aspen/LocalPtr.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"

namespace Aspen {

  /** Trait testing whether a type is a reactor. */
  template<typename T, typename=void>
  struct is_reactor : std::false_type {};

  template<typename T>
  struct is_reactor<T, std::enable_if_t<std::is_same_v<
    decltype(std::declval<T>().commit(std::declval<int>())), State>>> :
    std::true_type {};

  template<typename T>
  constexpr bool is_reactor_v = is_reactor<T>::value;

  /** Trait testing whether a type is a pointer reactor. */
  template<typename T>
  struct is_reactor_pointer : std::false_type {};

  template<typename T>
  struct is_reactor_pointer<const T> : is_reactor_pointer<T> {};

  template<typename T>
  struct is_reactor_pointer<T&> : is_reactor_pointer<T> {};

  template<typename T>
  struct is_reactor_pointer<T&&> : is_reactor_pointer<T> {};

  template<typename T>
  struct is_reactor_pointer<T*> : is_reactor<T> {};

  template<typename T>
  struct is_reactor_pointer<std::shared_ptr<T>> : is_reactor<T> {};

  template<typename T>
  struct is_reactor_pointer<std::unique_ptr<T>> : is_reactor<T> {};

  template<typename T>
  constexpr bool is_reactor_pointer_v = is_reactor_pointer<T>::value;

  /** Trait to test whether a type is a reactor or a pointer to a reactor. */
  template<typename T>
  struct is_reactor_or_pointer :
    std::disjunction<is_reactor<T>, is_reactor_pointer<T>> {};

  template<typename T>
  constexpr bool is_reactor_or_pointer_v = is_reactor_or_pointer<T>::value;

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

  /** Trait used to wrap a type into a reactor. */
  template<typename T, typename=void>
  struct to_reactor {
    using type = Constant<std::decay_t<T>>;
  };

  template<typename T>
  struct to_reactor<T, std::enable_if_t<
      is_reactor_or_pointer_v<std::decay_t<T>>>> {
    using type = std::decay_t<T>;
  };

  template<typename T>
  using to_reactor_t = typename to_reactor<T>::type;

  /** Trait used to determine what type a reactor evaluates to. */
  template<typename T>
  struct reactor_result {
    using type =
      typename dereference_reactor_t<to_reactor_t<std::decay_t<T>>>::Type;
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
    noexcept(std::declval<dereference_reactor_t<R>>().eval())> {};

  template<typename R>
  constexpr auto is_noexcept_reactor_v = is_noexcept_reactor<R>::value;

  template<typename T>
  auto make_ptr(T&& value) {
    if constexpr(is_reactor_pointer_v<std::decay_t<T>>) {
      return std::forward<T>(value);
    } else {
      return std::make_unique<to_reactor_t<T>>(std::forward<T>(value));
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

  template<typename R>
  auto try_eval(const R& reactor) noexcept {
    if constexpr(is_noexcept_reactor_v<R>) {
      return LocalPtr(reactor.eval());
    } else {
      return try_call([&] { return reactor.eval(); });
    }
  }
}

#endif

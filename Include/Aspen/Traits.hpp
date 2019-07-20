#ifndef ASPEN_TRAITS_HPP
#define ASPEN_TRAITS_HPP
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
}

#endif

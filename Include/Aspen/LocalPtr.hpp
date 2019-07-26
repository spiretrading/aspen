#ifndef ASPEN_LOCAL_PTR_HPP
#define ASPEN_LOCAL_PTR_HPP
#include <type_traits>
#include <utility>

namespace Aspen {

  /**
   * Wraps a value with pointer semantics, useful for providing a uniform
   * interface to values that may or may not be pointers.
   * @param <T> The type of value to wrap.
   */
  template<typename T>
  class LocalPtr {
    public:
      using Type = T;

      /**
       * Constructs a LocalPtr.
       * @param args Arguments used to initialize the value.
       */
      template<typename... A>
      constexpr explicit LocalPtr(A&&... args);

      /** Returns a pointer to the wrapped value. */
      constexpr Type* operator ->() const noexcept;

    private:
      mutable Type m_value;
  };

  /** Trait used to determine whether a type should be wrapped by a LocalPtr. */
  template<typename T, typename=void>
  struct try_ptr {
    using type = LocalPtr<T>;
  };

  template<typename T>
  struct try_ptr<T, std::void_t<decltype(*std::declval<T>())>> {
    using type = T;
  };

  template<typename T>
  using try_ptr_t = typename try_ptr<T>::type;

  template<typename T>
  template<typename... A>
  constexpr LocalPtr<T>::LocalPtr(A&&... value)
    : m_value(std::forward<A>(value)...) {}

  template<typename T>
  constexpr typename LocalPtr<T>::Type* LocalPtr<T>::operator ->()
      const noexcept {
    return &m_value;
  }
}

#endif

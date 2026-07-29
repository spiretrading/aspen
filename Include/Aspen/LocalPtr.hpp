#ifndef ASPEN_LOCAL_PTR_HPP
#define ASPEN_LOCAL_PTR_HPP
#include <concepts>
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

      /** The type of value to wrap. */
      using Type = T;

      /**
       * Constructs a LocalPtr.
       * @param args Arguments used to initialize the value.
       */
      template<typename... A> requires
        std::constructible_from<T, A...> && (sizeof...(A) != 1 ||
          (!std::same_as<std::remove_cvref_t<A>, LocalPtr<T>> && ...))
      constexpr explicit LocalPtr(A&&... args) noexcept(
        std::is_nothrow_constructible_v<T, A...>);

      constexpr LocalPtr(const LocalPtr&) = default;
      constexpr LocalPtr(LocalPtr&&) = default;

      template<typename S>
      constexpr auto& operator *(this S&& self) noexcept;
      template<typename S>
      constexpr auto* operator ->(this S&& self) noexcept;
      constexpr LocalPtr& operator =(const LocalPtr&) = default;
      constexpr LocalPtr& operator =(LocalPtr&&) = default;
      template<typename U> requires(
        !std::same_as<std::remove_cvref_t<U>, LocalPtr<T>>) &&
        std::is_assignable_v<T&, U>
      constexpr LocalPtr& operator =(U&& value) noexcept(
        std::is_nothrow_assignable_v<T&, U>);

    private:
      [[no_unique_address]]
      Type m_value;
  };

  template<typename T>
  LocalPtr(T&& value) -> LocalPtr<std::decay_t<T>>;

  /** Concept for a type that can be dereferenced. */
  template<typename T>
  concept Dereferenceable = requires { *std::declval<T>(); };

  /** Trait used to determine whether a type should be wrapped by a LocalPtr. */
  template<typename T>
  using try_ptr_t = std::conditional_t<Dereferenceable<T>, T, LocalPtr<T>>;

  template<typename T>
  template<typename... A> requires
    std::constructible_from<T, A...> && (sizeof...(A) != 1 ||
      (!std::same_as<std::remove_cvref_t<A>, LocalPtr<T>> && ...))
  constexpr LocalPtr<T>::LocalPtr(A&&... args) noexcept(
    std::is_nothrow_constructible_v<T, A...>)
    : m_value(std::forward<A>(args)...) {}

  template<typename T>
  template<typename S>
  constexpr auto& LocalPtr<T>::operator *(this S&& self) noexcept {
    return self.m_value;
  }

  template<typename T>
  template<typename S>
  constexpr auto* LocalPtr<T>::operator ->(this S&& self) noexcept {
    return &self.m_value;
  }

  template<typename T>
  template<typename U> requires(
    !std::same_as<std::remove_cvref_t<U>, LocalPtr<T>>) &&
    std::is_assignable_v<T&, U>
  constexpr LocalPtr<T>& LocalPtr<T>::operator =(U&& value) noexcept(
      std::is_nothrow_assignable_v<T&, U>) {
    m_value = std::forward<U>(value);
    return *this;
  }
}

#endif

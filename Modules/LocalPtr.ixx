module;
#include <type_traits>
#include <utility>

export module Aspen:LocalPtr;

export namespace Aspen {

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

      constexpr LocalPtr(const LocalPtr& args) = default;

      constexpr LocalPtr(LocalPtr&& args) = default;

      /** Implicitly converts to the underlying value. */
      constexpr operator const Type& () const;

      /** Implicitly converts to the underlying value. */
      constexpr operator Type& ();

      //! Returns a reference to the value.
      constexpr const Type& operator *() const noexcept;

      /** Returns a pointer to the wrapped value. */
      constexpr const Type* operator ->() const noexcept;

      //! Returns a reference to the value.
      constexpr Type& operator *() noexcept;

      /** Returns a pointer to the wrapped value. */
      constexpr Type* operator ->() noexcept;

      constexpr LocalPtr& operator =(const LocalPtr& ptr) = default;

      constexpr LocalPtr& operator =(LocalPtr&& ptr) = default;

    private:
      Type m_value;
  };

  template<typename T>
  LocalPtr(T&& value) -> LocalPtr<std::decay_t<T>>;

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
  constexpr LocalPtr<T>::operator const typename LocalPtr<T>::Type& () const {
    return m_value;
  }

  template<typename T>
  constexpr LocalPtr<T>::operator typename LocalPtr<T>::Type& () {
    return m_value;
  }

  template<typename T>
  constexpr const typename LocalPtr<T>::Type& LocalPtr<T>::operator *()
      const noexcept {
    return m_value;
  }

  template<typename T>
  constexpr const typename LocalPtr<T>::Type* LocalPtr<T>::operator ->()
      const noexcept {
    return &m_value;
  }

  template<typename T>
  constexpr typename LocalPtr<T>::Type& LocalPtr<T>::operator *() noexcept {
    return m_value;
  }

  template<typename T>
  constexpr typename LocalPtr<T>::Type* LocalPtr<T>::operator ->() noexcept {
    return &m_value;
  }
}

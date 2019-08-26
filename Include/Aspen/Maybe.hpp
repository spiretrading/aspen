#ifndef ASPEN_MAYBE_HPP
#define ASPEN_MAYBE_HPP
#include <exception>
#include <type_traits>
#include <utility>
#include <variant>
#include "Aspen/LocalPtr.hpp"

namespace Aspen {

  /**
   * Stores a value that could potentially result in an exception.
   * @param <T> The type of value to store.
   */
  template<typename T>
  class Maybe {
    public:
      using Type = T;

      /** Constructs a Maybe initialized to Type's default value. */
      Maybe() = default;

      /**
       * Constructs a Maybe initialized to a value.
       * @param value The value to store.
       */
      Maybe(const Type& value) noexcept(
        std::is_nothrow_copy_constructible_v<Type>);

      /**
       * Constructs a Maybe initialized to a value.
       * @param value The value to store.
       */
      Maybe(Type&& value) noexcept(
        std::is_nothrow_move_constructible_v<Type>);

      /**
       * Constructs a Maybe initialized to an exception.
       * @param exception The exception to throw.
       */
      Maybe(std::exception_ptr exception) noexcept;

      /**
       * Converts a Maybe of one type to this Maybe.
       * @param maybe The Maybe to convert.
       */
      template<typename U>
      Maybe(const Maybe<U>& maybe) noexcept(
        std::is_nothrow_constructible_v<Type, const U&>);

      /**
       * Moves a Maybe of one type to this Maybe.
       * @param maybe The Maybe to convert.
       */
      template<typename U>
      Maybe(Maybe<U>&& maybe) noexcept(
        std::is_nothrow_constructible_v<Type, U&&>);

      /** Implicitly converts to the underlying value. */
      operator const Type& () const;

      /** Implicitly converts to the underlying value. */
      operator Type& ();

      /** Returns <code>true</code> iff a value is stored. */
      bool has_value() const noexcept;

      /** Returns <code>true</code> iff an exception is stored. */
      bool has_exception() const noexcept;

      /** Returns the stored value, or throws an exception. */
      const Type& get() const;

      /** Returns the stored value, or throws an exception. */
      Type& get();

      /** Returns the exception. */
      std::exception_ptr get_exception() const noexcept;

      //! Returns a reference to the value.
      const Type& operator *() const;

      //! Returns a pointer to the value.
      const Type* operator ->() const;

      //! Returns a reference to the value.
      Type& operator *();

      //! Returns a pointer to the value.
      Type* operator ->();

      template<typename U>
      Maybe& operator =(const Maybe<U>& rhs) noexcept(
        std::is_nothrow_assignable_v<Type, const U&>);

      template<typename U>
      Maybe& operator =(Maybe<U>&& rhs) noexcept(
        std::is_nothrow_assignable_v<Type, U&&>);

      template<typename U>
      Maybe& operator =(const U& rhs) noexcept(
        std::is_nothrow_assignable_v<Type, const U&>);

      template<typename U>
      Maybe& operator =(U&& rhs) noexcept(
        std::is_nothrow_assignable_v<Type, U&&>);

    private:
      template<typename> friend class Maybe;
      std::variant<std::exception_ptr, Type> m_value;
  };

  template<>
  class Maybe<void> {
    public:
      using Type = void;

      Maybe() = default;

      Maybe(std::exception_ptr exception) noexcept;

      template<typename U>
      Maybe(const Maybe<U>& maybe) noexcept;

      bool has_exception() const noexcept;

      void get() const;

      std::exception_ptr get_exception() const noexcept;

      void operator *() const;

      void operator ->() const;

      template<typename U>
      Maybe& operator =(const Maybe<U>& rhs) noexcept;

    private:
      std::exception_ptr m_exception;
  };

  /**
   * Type trait that wraps a type in a Maybe depending on a condition.
   * @param <T> The type to potentially wrap.
   * @param <C> The condition used to test if <code>T</code> is wrapped.
   */
  template<typename T, bool C>
  struct try_maybe {
    using type = std::conditional_t<C, Maybe<T>, LocalPtr<T>>;
  };

  template<typename T, bool C>
  using try_maybe_t = typename try_maybe<T, C>::type;

  /**
   * Tries calling a function, capturing any thrown exception.
   * @param f The function to call.
   * @return The result of <i>f</i>.
   */
  template<typename F>
  decltype(auto) try_call(F&& f) noexcept(noexcept(f())) {
    if constexpr(noexcept(f())) {
      return f();
    } else {
      using Type = std::decay_t<std::invoke_result_t<F>>;
      try {
        if constexpr(std::is_same_v<Type, void>) {
          f();
          return Maybe<Type>();
        } else {
          return Maybe<Type>(f());
        }
      } catch(...) {
        return Maybe<Type>(std::current_exception());
      }
    }
  }

  template<typename T>
  Maybe<T>::Maybe(const Type& value) noexcept(
    std::is_nothrow_copy_constructible_v<Type>)
    : m_value(value) {}

  template<typename T>
  Maybe<T>::Maybe(Type&& value) noexcept(
    std::is_nothrow_move_constructible_v<Type>)
    : m_value(std::move(value)) {}

  template<typename T>
  Maybe<T>::Maybe(std::exception_ptr exception) noexcept
    : m_value(std::move(exception)) {}

  template<typename T>
  template<typename U>
  Maybe<T>::Maybe(const Maybe<U>& maybe) noexcept(
    std::is_nothrow_constructible_v<Type, const U&>)
    : m_value([&] () -> std::variant<std::exception_ptr, Type> {
        if(maybe.has_value()) {
          return static_cast<const U&>(maybe);
        } else {
          return maybe.get_exception();
        }
      }()) {}

  template<typename T>
  template<typename U>
  Maybe<T>::Maybe(Maybe<U>&& maybe) noexcept(
    std::is_nothrow_constructible_v<Type, U&&>)
    : m_value([&] () -> std::variant<std::exception_ptr, Type> {
        if(maybe.has_value()) {
          return std::move(static_cast<U&>(maybe));
        } else {
          return maybe.get_exception();
        }
      }()) {}

  template<typename T>
  Maybe<T>::operator const typename Maybe<T>::Type& () const {
    return get();
  }

  template<typename T>
  Maybe<T>::operator typename Maybe<T>::Type& () {
    return get();
  }

  template<typename T>
  bool Maybe<T>::has_value() const noexcept {
    return m_value.index() == 1;
  }

  template<typename T>
  bool Maybe<T>::has_exception() const noexcept {
    return m_value.index() == 0;
  }

  template<typename T>
  const typename Maybe<T>::Type& Maybe<T>::get() const {
    if(has_value()) {
      return std::get<Type>(m_value);
    }
    auto& e = std::get<std::exception_ptr>(m_value);
    if(e != nullptr) {
      std::rethrow_exception(e);
    }
    throw std::runtime_error("Uninitialized.");
  }

  template<typename T>
  typename Maybe<T>::Type& Maybe<T>::get() {
    if(has_value()) {
      return std::get<Type>(m_value);
    }
    auto& e = std::get<std::exception_ptr>(m_value);
    if(e != nullptr) {
      std::rethrow_exception(e);
    }
    throw std::runtime_error("Uninitialized.");
  }

  template<typename T>
  std::exception_ptr Maybe<T>::get_exception() const noexcept {
    if(has_exception()) {
      return std::get<std::exception_ptr>(m_value);
    }
    return {};
  }

  template<typename T>
  const typename Maybe<T>::Type& Maybe<T>::operator *() const {
    return get();
  }

  template<typename T>
  const typename Maybe<T>::Type* Maybe<T>::operator ->() const {
    return &get();
  }

  template<typename T>
  typename Maybe<T>::Type& Maybe<T>::operator *() {
    return get();
  }

  template<typename T>
  typename Maybe<T>::Type* Maybe<T>::operator ->() {
    return &get();
  }

  template<typename T>
  template<typename U>
  Maybe<T>& Maybe<T>::operator =(const Maybe<U>& rhs) noexcept(
      std::is_nothrow_assignable_v<Type, const U&>) {
    m_value = rhs.m_value;
    return *this;
  }

  template<typename T>
  template<typename U>
  Maybe<T>& Maybe<T>::operator =(Maybe<U>&& rhs) noexcept(
      std::is_nothrow_assignable_v<Type, U&&>) {
    m_value = std::move(rhs.m_value);
    return *this;
  }

  template<typename T>
  template<typename U>
  Maybe<T>& Maybe<T>::operator =(const U& rhs) noexcept(
      std::is_nothrow_assignable_v<Type, const U&>) {
    m_value = rhs;
    return *this;
  }

  template<typename T>
  template<typename U>
  Maybe<T>& Maybe<T>::operator =(U&& rhs) noexcept(
      std::is_nothrow_assignable_v<Type, U&&>) {
    m_value = std::move(rhs);
    return *this;
  }

  inline Maybe<void>::Maybe(std::exception_ptr exception) noexcept
    : m_exception(std::move(exception)) {}

  template<typename U>
  Maybe<void>::Maybe(const Maybe<U>& maybe) noexcept {
    if(maybe.has_exception()) {
      m_exception = maybe.get_exception();
    }
  }

  inline bool Maybe<void>::has_exception() const noexcept {
    return m_exception != nullptr;
  }

  inline void Maybe<void>::get() const {
    if(m_exception != nullptr) {
      std::rethrow_exception(m_exception);
    }
  }

  inline std::exception_ptr Maybe<void>::get_exception() const noexcept {
    return m_exception;
  }

  inline void Maybe<void>::operator *() const {
    get();
  }

  inline void Maybe<void>::operator ->() const {
    get();
  }

  template<typename U>
  Maybe<void>& Maybe<void>::operator =(const Maybe<U>& rhs) noexcept {
    if(rhs.has_exception()) {
      m_exception = rhs.get_exception();
    } else {
      m_exception = nullptr;
    }
    return *this;
  }
}

#endif

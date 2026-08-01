#ifndef ASPEN_MAYBE_HPP
#define ASPEN_MAYBE_HPP
#include <concepts>
#include <exception>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>
#include "Aspen/LocalPtr.hpp"

namespace Aspen {
  template<typename T>
  class Maybe;

namespace Details {
  template<typename T>
  struct is_maybe : std::false_type {};

  template<typename T>
  struct is_maybe<Maybe<T>> : std::true_type {};
}

  /** Concept for a type that is a Maybe. */
  template<typename T>
  concept IsMaybe = Details::is_maybe<std::remove_cvref_t<T>>::value;

  /**
   * Stores a value that could potentially result in an exception.
   * @param <T> The type of value to store.
   */
  template<typename T>
  class Maybe {
    public:

      /** The type of value to store. */
      using Type = T;

      /** Constructs a Maybe storing neither a value nor an exception. */
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
      template<typename U> requires std::constructible_from<T, const U&>
      explicit(!std::convertible_to<const U&, T>) Maybe(
          const Maybe<U>& maybe) noexcept(
            std::is_nothrow_constructible_v<Type, const U&>)
          : m_value([&] () -> std::variant<std::exception_ptr, Type> {
              if(maybe.has_value()) {
                return static_cast<Type>(maybe.get());
              } else {
                return maybe.get_exception();
              }
            }()) {}

      /**
       * Moves a Maybe of one type to this Maybe.
       * @param maybe The Maybe to convert.
       */
      template<typename U> requires std::constructible_from<T, U&&>
      explicit(!std::convertible_to<U&&, T>) Maybe(Maybe<U>&& maybe) noexcept(
          std::is_nothrow_constructible_v<Type, U&&>)
          : m_value([&] () -> std::variant<std::exception_ptr, Type> {
              if(maybe.has_value()) {
                return static_cast<Type>(std::move(maybe.get()));
              } else {
                return maybe.get_exception();
              }
            }()) {}

      Maybe(const Maybe&) = default;
      Maybe(Maybe&&) = default;

      /** Returns <code>true</code> iff a value is stored. */
      bool has_value() const noexcept;

      /** Returns <code>true</code> iff an exception is stored. */
      bool has_exception() const noexcept;

      /** Returns the stored value, or throws an exception. */
      auto&& get(this auto&& self);

      /** Returns the exception. */
      std::exception_ptr get_exception() const noexcept;

      operator const Type& () const;
      operator Type& ();
      auto&& operator *(this auto&& self);
      auto* operator ->(this auto&& self);
      Maybe& operator =(const Maybe&) = default;
      Maybe& operator =(Maybe&&) = default;
      template<typename U> requires std::constructible_from<T, const U&>
      Maybe& operator =(const Maybe<U>& maybe) noexcept(
        std::is_nothrow_assignable_v<Type&, const U&> &&
        std::is_nothrow_constructible_v<Type, const U&>);
      template<typename U> requires std::constructible_from<T, U&&>
      Maybe& operator =(Maybe<U>&& maybe) noexcept(
        std::is_nothrow_assignable_v<Type&, U&&> &&
        std::is_nothrow_constructible_v<Type, U&&>);
      template<typename U> requires(!IsMaybe<U>) &&
        std::is_assignable_v<std::variant<std::exception_ptr, T>&, U>
      Maybe& operator =(U&& value) noexcept(
        std::is_nothrow_assignable_v<std::variant<std::exception_ptr, T>&, U>);

    private:
      template<typename> friend class Maybe;
      std::variant<std::exception_ptr, Type> m_value;
  };

  /** Stores an exception resulting from an operation with no value. */
  template<>
  class Maybe<void> {
    public:

      /** The type of value to store. */
      using Type = void;

      /** Constructs a Maybe storing no exception. */
      Maybe() = default;

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
      Maybe(const Maybe<U>& maybe) noexcept;

      Maybe(const Maybe&) = default;
      Maybe(Maybe&&) = default;

      /** Returns <code>true</code> iff an exception is stored. */
      bool has_exception() const noexcept;

      /** Throws the stored exception, if one is stored. */
      void get() const;

      /** Returns the exception. */
      std::exception_ptr get_exception() const noexcept;

      void operator *() const;
      void operator ->() const;
      Maybe& operator =(const Maybe&) = default;
      Maybe& operator =(Maybe&&) = default;
      template<typename U>
      Maybe& operator =(const Maybe<U>& maybe) noexcept;

    private:
      std::exception_ptr m_exception;
  };

  Maybe(std::exception_ptr) -> Maybe<void>;

  /**
   * Type trait that wraps a type in a Maybe depending on a condition.
   * @param <T> The type to potentially wrap.
   * @param <C> The condition used to test if <code>T</code> is wrapped.
   */
  template<typename T, bool C>
  struct try_maybe {
    using type = std::conditional_t<C, Maybe<T>,
      std::conditional_t<std::is_same_v<T, void>, Maybe<void>,
        std::conditional_t<std::is_default_constructible_v<T>, LocalPtr<T>,
          std::optional<T>>>>;
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
    using Type = std::decay_t<std::invoke_result_t<F>>;
    if constexpr(noexcept(f())) {
      if constexpr(std::is_same_v<Type, void>) {
        f();
        return Maybe<void>();
      } else {
        return f();
      }
    } else {
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
  bool Maybe<T>::has_value() const noexcept {
    return m_value.index() == 1;
  }

  template<typename T>
  bool Maybe<T>::has_exception() const noexcept {
    return m_value.index() == 0;
  }

  template<typename T>
  auto&& Maybe<T>::get(this auto&& self) {
    if(self.has_value()) {
      return std::get<Type>(std::forward<decltype(self)>(self).m_value);
    }
    auto& exception = std::get<std::exception_ptr>(self.m_value);
    if(exception) {
      std::rethrow_exception(exception);
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
  Maybe<T>::operator const T& () const {
    return get();
  }

  template<typename T>
  Maybe<T>::operator T& () {
    return get();
  }

  template<typename T>
  auto&& Maybe<T>::operator *(this auto&& self) {
    return std::forward<decltype(self)>(self).get();
  }

  template<typename T>
  auto* Maybe<T>::operator ->(this auto&& self) {
    return &self.get();
  }

  template<typename T>
  template<typename U> requires std::constructible_from<T, const U&>
  Maybe<T>& Maybe<T>::operator =(const Maybe<U>& maybe) noexcept(
      std::is_nothrow_assignable_v<Type&, const U&> &&
      std::is_nothrow_constructible_v<Type, const U&>) {
    if(!maybe.has_value()) {
      m_value = maybe.get_exception();
    } else if constexpr(std::is_assignable_v<Type&, const U&>) {
      if(has_value()) {
        std::get<Type>(m_value) = maybe.get();
      } else {
        m_value.template emplace<Type>(maybe.get());
      }
    } else {
      m_value = static_cast<Type>(maybe.get());
    }
    return *this;
  }

  template<typename T>
  template<typename U> requires std::constructible_from<T, U&&>
  Maybe<T>& Maybe<T>::operator =(Maybe<U>&& maybe) noexcept(
      std::is_nothrow_assignable_v<Type&, U&&> &&
      std::is_nothrow_constructible_v<Type, U&&>) {
    if(!maybe.has_value()) {
      m_value = maybe.get_exception();
    } else if constexpr(std::is_assignable_v<Type&, U&&>) {
      if(has_value()) {
        std::get<Type>(m_value) = std::move(maybe.get());
      } else {
        m_value.template emplace<Type>(std::move(maybe.get()));
      }
    } else {
      m_value = static_cast<Type>(std::move(maybe.get()));
    }
    return *this;
  }

  template<typename T>
  template<typename U> requires(!IsMaybe<U>) &&
    std::is_assignable_v<std::variant<std::exception_ptr, T>&, U>
  Maybe<T>& Maybe<T>::operator =(U&& value) noexcept(
      std::is_nothrow_assignable_v<std::variant<std::exception_ptr, T>&, U>) {
    m_value = std::forward<U>(value);
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
    return static_cast<bool>(m_exception);
  }

  inline void Maybe<void>::get() const {
    if(m_exception) {
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
  Maybe<void>& Maybe<void>::operator =(const Maybe<U>& maybe) noexcept {
    if(maybe.has_exception()) {
      m_exception = maybe.get_exception();
    } else {
      m_exception = nullptr;
    }
    return *this;
  }
}

#endif

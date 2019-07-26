#ifndef ASPEN_MAYBE_HPP
#define ASPEN_MAYBE_HPP
#include <exception>
#include <type_traits>
#include <utility>
#include <variant>

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
      Maybe(const Type& value);

      /**
       * Constructs a Maybe initialized to a value.
       * @param value The value to store.
       */
      Maybe(Type&& value);

      /**
       * Constructs a Maybe initialized to an exception.
       * @param exception The exception to throw.
       */
      Maybe(std::exception_ptr exception);

      /**
       * Converts a Maybe of one type to this Maybe.
       * @param maybe The Maybe to convert.
       */
      template<typename U>
      Maybe(const Maybe<U>& maybe);

      /**
       * Moves a Maybe of one type to this Maybe.
       * @param maybe The Maybe to convert.
       */
      template<typename U>
      Maybe(Maybe<U>&& maybe);

      /** Implicitly converts to the underlying value. */
      operator const Type& () const;

      /** Implicitly converts to the underlying value. */
      operator Type& ();

      /** Returns <code>true</code> iff a value is stored. */
      bool has_value() const;

      /** Returns <code>true</code> iff an exception is stored. */
      bool has_exception() const;

      /** Returns the stored value, or throws an exception. */
      const Type& get() const;

      /** Returns the stored value, or throws an exception. */
      Type& get();

      /** Returns the exception. */
      std::exception_ptr get_exception() const;

      //! Returns a reference to the value.
      const Type& operator *() const;

      //! Returns a pointer to the value.
      const Type* operator ->() const;

      //! Returns a reference to the value.
      Type& operator *();

      //! Returns a pointer to the value.
      Type* operator ->();

      template<typename U>
      Maybe& operator =(const Maybe<U>& rhs);

      template<typename U>
      Maybe& operator =(Maybe<U>&& rhs);

      template<typename U>
      Maybe& operator =(const U& rhs);

      template<typename U>
      Maybe& operator =(U&& rhs);

    private:
      template<typename> friend class Maybe;
      std::variant<Type, std::exception_ptr> m_value;
  };

  template<>
  class Maybe<void> {
    public:
      using Type = void;

      Maybe() = default;

      Maybe(std::exception_ptr exception);

      template<typename U>
      Maybe(const Maybe<U>& maybe);

      bool has_exception() const;

      void get() const;

      std::exception_ptr get_exception() const;

      void operator *() const;

      void operator ->() const;

      template<typename U>
      Maybe& operator =(const Maybe<U>& rhs);

    private:
      std::exception_ptr m_exception;
  };

  /**
   * Tries calling a function, capturing any thrown exception.
   * @param f The function to call.
   * @return The result of <i>f</i>.
   */
  template<typename F>
  decltype(auto) try_call(F&& f) {
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
  Maybe<T>::Maybe(const Type& value)
    : m_value(value) {}

  template<typename T>
  Maybe<T>::Maybe(Type&& value)
    : m_value(std::move(value)) {}

  template<typename T>
  Maybe<T>::Maybe(std::exception_ptr exception)
    : m_value(std::move(exception)) {}

  template<typename T>
  template<typename U>
  Maybe<T>::Maybe(const Maybe<U>& maybe)
    : m_value([&] () -> std::variant<Type, std::exception_ptr> {
        if(maybe.has_value()) {
          return static_cast<const U&>(maybe);
        } else {
          return maybe.get_exception();
        }
      }()) {}

  template<typename T>
  template<typename U>
  Maybe<T>::Maybe(Maybe<U>&& maybe)
    : m_value([&] () -> std::variant<Type, std::exception_ptr> {
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
  bool Maybe<T>::has_value() const {
    return m_value.index() == 0;
  }

  template<typename T>
  bool Maybe<T>::has_exception() const {
    return m_value.index() == 1;
  }

  template<typename T>
  const typename Maybe<T>::Type& Maybe<T>::get() const {
    if(has_value()) {
      return std::get<Type>(m_value);
    }
    std::rethrow_exception(std::get<std::exception_ptr>(m_value));
    throw std::exception();
  }

  template<typename T>
  typename Maybe<T>::Type& Maybe<T>::get() {
    if(has_value()) {
      return std::get<Type>(m_value);
    }
    std::rethrow_exception(std::get<std::exception_ptr>(m_value));
    throw std::exception();
  }

  template<typename T>
  std::exception_ptr Maybe<T>::get_exception() const {
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
  Maybe<T>& Maybe<T>::operator =(const Maybe<U>& rhs) {
    m_value = rhs.m_value;
    return *this;
  }

  template<typename T>
  template<typename U>
  Maybe<T>& Maybe<T>::operator =(Maybe<U>&& rhs) {
    m_value = std::move(rhs.m_value);
    return *this;
  }

  template<typename T>
  template<typename U>
  Maybe<T>& Maybe<T>::operator =(const U& rhs) {
    m_value = rhs;
    return *this;
  }

  template<typename T>
  template<typename U>
  Maybe<T>& Maybe<T>::operator =(U&& rhs) {
    m_value = std::move(rhs);
    return *this;
  }

  inline Maybe<void>::Maybe(std::exception_ptr exception)
    : m_exception(std::move(exception)) {}

  template<typename U>
  Maybe<void>::Maybe(const Maybe<U>& maybe) {
    if(maybe.has_exception()) {
      m_exception = maybe.get_exception();
    }
  }

  inline bool Maybe<void>::has_exception() const {
    return m_exception != nullptr;
  }

  inline void Maybe<void>::get() const {
    if(m_exception != nullptr) {
      std::rethrow_exception(m_exception);
    }
  }

  inline std::exception_ptr Maybe<void>::get_exception() const {
    return m_exception;
  }

  inline void Maybe<void>::operator *() const {
    get();
  }

  inline void Maybe<void>::operator ->() const {
    get();
  }

  template<typename U>
  Maybe<void>& Maybe<void>::operator =(const Maybe<U>& rhs) {
    if(rhs.has_exception()) {
      m_exception = rhs.get_exception();
    } else {
      m_exception = nullptr;
    }
    return *this;
  }
}

#endif

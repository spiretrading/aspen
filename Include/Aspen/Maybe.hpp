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

      /** Implicitly converts to the underlying value. */
      operator const Type& () const;

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

      bool has_exception() const;

      void get() const;

      std::exception_ptr get_exception() const;

    private:
      std::exception_ptr m_exception;
  };

  /**
   * Tries calling a function, capturing any thrown exception.
   * @param f The function to call.
   * @return The result of <i>f</i>.
   */
  template<typename F>
  Maybe<std::invoke_result_t<F>> try_call(F&& f) {
    try {
      if constexpr(std::is_same_v<std::invoke_result_t<F>, void>) {
        f();
        return {};
      } else {
        return f();
      }
    } catch(...) {
      return std::current_exception();
    }
  };

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
  Maybe<T>::operator const typename Maybe<T>::Type& () const {
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
}

#endif

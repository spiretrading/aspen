#ifndef ASPEN_REACTOR_TESTS_HPP
#define ASPEN_REACTOR_TESTS_HPP
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"

namespace Aspen::Tests {

  /**
   * Stores a value and counts how many values of its type are copied and
   * destroyed.
   */
  class CountedValue {
    public:

      /** Returns the number of copies counted since the last reset. */
      static std::size_t get_copies() noexcept;

      /** Returns the number of destructions counted since the last reset. */
      static std::size_t get_destructions() noexcept;

      /** Sets all of the counts to zero. */
      static void reset_counts() noexcept;

      /** Constructs a CountedValue storing zero. */
      CountedValue() noexcept;

      /**
       * Constructs a CountedValue.
       * @param value The value to store.
       */
      explicit CountedValue(int value) noexcept;

      CountedValue(const CountedValue& value) noexcept;
      CountedValue(CountedValue&& value) noexcept;
      ~CountedValue();

      /** Returns the stored value. */
      int get_value() const noexcept;

      CountedValue& operator =(const CountedValue& value) noexcept;
      CountedValue& operator =(CountedValue&& value) noexcept;

    private:
      static inline auto m_copies = std::size_t(0);
      static inline auto m_destructions = std::size_t(0);
      int m_value;
  };

  /**
   * Implements a reactor returning its evaluation by value.
   * @param <T> The type of value to evaluate to.
   */
  template<typename T>
  class ByValueReactor {
    public:

      /** The type to evaluate to. */
      using Type = T;

      /**
       * Constructs a ByValueReactor.
       * @param value The value to evaluate to.
       */
      explicit ByValueReactor(T value) noexcept;

      State commit(std::uint64_t sequence) noexcept;
      Type eval() const;

    private:
      T m_value;
  };

  template<typename T>
  ByValueReactor(T&&) -> ByValueReactor<std::decay_t<T>>;

  /**
   * Implements a reactor evaluating to a fixed value whose destruction can be
   * observed.
   * @param <T> The type of value to evaluate to.
   */
  template<typename T>
  class TrackedReactor {
    public:

      /** The type to evaluate to. */
      using Type = T;

      /**
       * Constructs a TrackedReactor.
       * @param value The value to evaluate to.
       * @param state The state to report from every commit.
       */
      TrackedReactor(T value, State state);

      /** Returns a token that expires when this reactor is destroyed. */
      std::weak_ptr<void> get_token() const noexcept;

      State commit(std::uint64_t sequence) noexcept;
      const Type& eval() const;

    private:
      std::shared_ptr<void> m_token;
      T m_value;
      State m_state;
  };

  /**
   * Implements a reactor counting how many times it is committed.
   * @param <T> The type of value to evaluate to.
   */
  template<typename T>
  class CountingReactor {
    public:

      /** The type to evaluate to. */
      using Type = T;

      /**
       * Constructs a CountingReactor.
       * @param value The value to evaluate to.
       * @param state The state to report from every commit.
       */
      CountingReactor(T value, State state);

      /** Returns the number of commits performed. */
      int get_commits() const noexcept;

      State commit(std::uint64_t sequence) noexcept;
      const Type& eval() const;

    private:
      std::shared_ptr<int> m_commits;
      T m_value;
      State m_state;
  };

  /**
   * Tests that a reactor whose children evaluate by value evaluates to the
   * expected value without destroying it.
   * @param reactor The reactor to commit and evaluate.
   * @param expected The value the reactor is expected to evaluate to.
   */
  template<IsReactor R>
  void test_by_value_evaluation(R& reactor, int expected) {
    auto state = reactor.commit(0);
    REQUIRE(has_evaluation(state));
    CountedValue::reset_counts();
    const auto& value = reactor.eval();
    REQUIRE(value.get_value() == expected);
    REQUIRE(CountedValue::get_destructions() == 0);
  }

  inline std::size_t CountedValue::get_copies() noexcept {
    return m_copies;
  }

  inline std::size_t CountedValue::get_destructions() noexcept {
    return m_destructions;
  }

  inline void CountedValue::reset_counts() noexcept {
    m_copies = 0;
    m_destructions = 0;
  }

  inline CountedValue::CountedValue() noexcept
    : m_value(0) {}

  inline CountedValue::CountedValue(int value) noexcept
    : m_value(value) {}

  inline CountedValue::CountedValue(const CountedValue& value) noexcept
      : m_value(value.m_value) {
    ++m_copies;
  }

  inline CountedValue::CountedValue(CountedValue&& value) noexcept
      : m_value(value.m_value) {
    ++m_copies;
  }

  inline CountedValue::~CountedValue() {
    ++m_destructions;
  }

  inline int CountedValue::get_value() const noexcept {
    return m_value;
  }

  inline CountedValue& CountedValue::operator =(
      const CountedValue& value) noexcept {
    m_value = value.m_value;
    return *this;
  }

  inline CountedValue& CountedValue::operator =(
      CountedValue&& value) noexcept {
    m_value = value.m_value;
    return *this;
  }

  template<typename T>
  ByValueReactor<T>::ByValueReactor(T value) noexcept
    : m_value(std::move(value)) {}

  template<typename T>
  State ByValueReactor<T>::commit(std::uint64_t sequence) noexcept {
    return State::COMPLETE_EVALUATED;
  }

  template<typename T>
  typename ByValueReactor<T>::Type ByValueReactor<T>::eval() const {
    return m_value;
  }

  template<typename T>
  TrackedReactor<T>::TrackedReactor(T value, State state)
    : m_token(std::make_shared<int>(0)),
      m_value(std::move(value)),
      m_state(state) {}

  template<typename T>
  std::weak_ptr<void> TrackedReactor<T>::get_token() const noexcept {
    return m_token;
  }

  template<typename T>
  State TrackedReactor<T>::commit(std::uint64_t sequence) noexcept {
    return m_state;
  }

  template<typename T>
  const typename TrackedReactor<T>::Type& TrackedReactor<T>::eval() const {
    return m_value;
  }

  template<typename T>
  CountingReactor<T>::CountingReactor(T value, State state)
    : m_commits(std::make_shared<int>(0)),
      m_value(std::move(value)),
      m_state(state) {}

  template<typename T>
  int CountingReactor<T>::get_commits() const noexcept {
    return *m_commits;
  }

  template<typename T>
  State CountingReactor<T>::commit(std::uint64_t sequence) noexcept {
    ++*m_commits;
    return m_state;
  }

  template<typename T>
  const typename CountingReactor<T>::Type& CountingReactor<T>::eval() const {
    return m_value;
  }
}

#endif

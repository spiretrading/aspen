#ifndef ASPEN_REACTOR_TESTS_HPP
#define ASPEN_REACTOR_TESTS_HPP
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"

namespace Aspen::Tests {

  /** Stores a value and counts how many values of its type are copied and
      destroyed. */
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
   * Tests that committing and evaluating a reactor does not evaluate to a
   * value that has already been destroyed.
   * @param reactor The reactor to commit and evaluate.
   */
  template<IsReactor R>
  void test_evaluation_lifetime(R& reactor) {
    auto state = reactor.commit(0);
    REQUIRE(has_evaluation(state));
    CountedValue::reset_counts();
    [[maybe_unused]] const auto& value = reactor.eval();
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
}

#endif

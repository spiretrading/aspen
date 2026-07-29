#include <stdexcept>
#include <type_traits>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Maybe.hpp"

using namespace Aspen;

namespace {

  /** Records whether it has been moved from. */
  struct Tracker {
    bool m_is_moved;
    int m_value;

    explicit Tracker(int value)
      : m_is_moved(false),
        m_value(value) {}

    Tracker(const Tracker& tracker)
      : m_is_moved(false),
        m_value(tracker.m_value) {}

    Tracker(Tracker&& tracker) noexcept
        : m_is_moved(false),
          m_value(tracker.m_value) {
      tracker.m_is_moved = true;
    }

    Tracker& operator =(const Tracker& tracker) {
      m_value = tracker.m_value;
      return *this;
    }

    Tracker& operator =(Tracker&& tracker) noexcept {
      m_value = tracker.m_value;
      tracker.m_is_moved = true;
      return *this;
    }
  };
}

TEST_SUITE("Maybe") {
  TEST_CASE("empty_maybe") {
    auto maybe = Maybe<int>();
    REQUIRE(maybe.has_exception());
    REQUIRE(!maybe.has_value());
    REQUIRE(!maybe.get_exception());
    REQUIRE_THROWS_AS(maybe.get(), std::runtime_error);
  }

  TEST_CASE("value_maybe") {
    auto maybe = Maybe(123);
    REQUIRE(maybe.has_value());
    REQUIRE(!maybe.has_exception());
    REQUIRE(maybe.get() == 123);
    REQUIRE(maybe == 123);
  }

  TEST_CASE("exception_maybe") {
    auto maybe = Maybe<int>(std::make_exception_ptr(std::runtime_error("")));
    REQUIRE(!maybe.has_value());
    REQUIRE(maybe.has_exception());
    REQUIRE_THROWS_AS(maybe.get(), std::runtime_error);
    REQUIRE_THROWS_AS(void(maybe == 123), std::runtime_error);
    REQUIRE(maybe.get_exception());
  }

  TEST_CASE("converting_maybe_exception") {
    auto maybe_int = Maybe<int>(
      std::make_exception_ptr(std::runtime_error("")));
    auto maybe_double = Maybe<double>(maybe_int);
    REQUIRE(maybe_double.has_exception());
  }

  TEST_CASE("assigning_a_value_does_not_move_an_lvalue") {
    auto value = Tracker(1);
    auto maybe = Maybe(Tracker(2));
    maybe = value;
    REQUIRE(!value.m_is_moved);
    REQUIRE(maybe.get().m_value == 1);
    maybe = Tracker(3);
    REQUIRE(maybe.get().m_value == 3);
  }

  TEST_CASE("assigning_a_maybe_does_not_move_an_lvalue") {
    auto source = Maybe(Tracker(1));
    auto destination = Maybe(Tracker(2));
    destination = source;
    REQUIRE(!source.get().m_is_moved);
    REQUIRE(destination.get().m_value == 1);
    destination = std::move(source);
    REQUIRE(source.get().m_is_moved);
  }

  TEST_CASE("assigning_a_maybe_holding_an_exception") {
    auto source = Maybe<int>(std::make_exception_ptr(std::runtime_error("")));
    auto destination = Maybe(123);
    destination = source;
    REQUIRE(destination.has_exception());
    REQUIRE_THROWS_AS(destination.get(), std::runtime_error);
  }

  TEST_CASE("assigning_a_maybe_of_another_type") {
    auto source = Maybe(123);
    auto destination = Maybe(1.5);
    destination = source;
    REQUIRE(destination.get() == 123.0);
    auto failed = Maybe<int>(std::make_exception_ptr(std::runtime_error("")));
    destination = failed;
    REQUIRE(destination.has_exception());
  }

  TEST_CASE("assignment_is_noexcept_when_the_value_is") {
    REQUIRE((std::is_nothrow_assignable_v<Maybe<int>&, int>));
    REQUIRE((std::is_nothrow_assignable_v<Maybe<int>&, const int&>));
    REQUIRE(!(std::is_nothrow_assignable_v<Maybe<Tracker>&, const Tracker&>));
    REQUIRE((std::is_nothrow_assignable_v<Maybe<Tracker>&, Tracker>));
  }

  TEST_CASE("dereferencing_propagates_constness") {
    auto maybe = Maybe(123);
    REQUIRE((std::is_same_v<decltype(*maybe), int&>));
    REQUIRE((std::is_same_v<decltype(maybe.get()), int&>));
    REQUIRE((std::is_same_v<decltype(maybe.operator ->()), int*>));
    const auto& constant = maybe;
    REQUIRE((std::is_same_v<decltype(*constant), const int&>));
    REQUIRE((std::is_same_v<decltype(constant.get()), const int&>));
    REQUIRE((std::is_same_v<decltype(constant.operator ->()), const int*>));
    *maybe = 7;
    REQUIRE(constant.get() == 7);
  }

  TEST_CASE("identifying_a_maybe") {
    REQUIRE(IsMaybe<Maybe<int>>);
    REQUIRE(IsMaybe<const Maybe<int>&>);
    REQUIRE(IsMaybe<Maybe<void>>);
    REQUIRE(!IsMaybe<int>);
    REQUIRE(!IsMaybe<LocalPtr<int>>);
  }

  TEST_CASE("void_maybe") {
    auto maybe = Maybe<void>();
    REQUIRE(!maybe.has_exception());
    REQUIRE(!maybe.get_exception());
    maybe.get();
    auto failed = Maybe<void>(std::make_exception_ptr(std::runtime_error("")));
    REQUIRE(failed.has_exception());
    REQUIRE_THROWS_AS(failed.get(), std::runtime_error);
    maybe = failed;
    REQUIRE(maybe.has_exception());
    maybe = Maybe<void>();
    REQUIRE(!maybe.has_exception());
  }
}

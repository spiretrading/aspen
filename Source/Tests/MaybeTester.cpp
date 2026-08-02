#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/LocalPtr.hpp"
#include "Aspen/Maybe.hpp"

using namespace Aspen;

namespace {
  struct Required {
    int m_value;

    explicit Required(int value)
      : m_value(value) {}
  };

  struct Guarded {
    int m_value;

    explicit Guarded(int value)
        : m_value(value) {
      if(value < 0) {
        throw std::runtime_error("negative");
      }
    }

    Guarded& operator =(int value) noexcept {
      m_value = value;
      return *this;
    }
  };

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
  TEST_CASE("default_construction") {
    auto maybe = Maybe<int>();
    REQUIRE(!maybe.has_exception());
    REQUIRE(!maybe.has_value());
    REQUIRE(!maybe.get_exception());
    REQUIRE_THROWS_AS(maybe.get(), std::runtime_error);
  }

  TEST_CASE("value") {
    auto maybe = Maybe(123);
    REQUIRE(maybe.has_value());
    REQUIRE(!maybe.has_exception());
    REQUIRE(maybe.get() == 123);
    REQUIRE(maybe == 123);
  }

  TEST_CASE("exception") {
    auto maybe = Maybe<int>(std::make_exception_ptr(std::runtime_error("")));
    REQUIRE(!maybe.has_value());
    REQUIRE(maybe.has_exception());
    REQUIRE_THROWS_AS(maybe.get(), std::runtime_error);
    REQUIRE_THROWS_AS(void(maybe == 123), std::runtime_error);
    REQUIRE(maybe.get_exception());
  }

  TEST_CASE("converting_an_exception") {
    auto maybe_int = Maybe<int>(
      std::make_exception_ptr(std::runtime_error("")));
    auto maybe_double = Maybe<double>(maybe_int);
    REQUIRE(maybe_double.has_exception());
  }

  TEST_CASE("assign_from_an_lvalue") {
    auto value = Tracker(1);
    auto maybe = Maybe(Tracker(2));
    maybe = value;
    REQUIRE(!value.m_is_moved);
    REQUIRE(maybe.get().m_value == 1);
    maybe = Tracker(3);
    REQUIRE(maybe.get().m_value == 3);
  }

  TEST_CASE("assign_from_another_type") {
    auto source = Maybe(123);
    auto destination = Maybe(1.5);
    destination = source;
    REQUIRE(destination.get() == 123.0);
    auto failed = Maybe<int>(std::make_exception_ptr(std::runtime_error("")));
    destination = failed;
    REQUIRE(destination.has_exception());
  }

  TEST_CASE("assign_from_another_type_rvalue") {
    auto destination = Maybe(1.5);
    destination = Maybe(123);
    REQUIRE(destination.get() == 123.0);
    destination = Maybe<int>(std::make_exception_ptr(std::runtime_error("")));
    REQUIRE(destination.has_exception());
    REQUIRE_THROWS_AS(destination.get(), std::runtime_error);
  }

  TEST_CASE("assignment_in_place") {
    auto maybe = Maybe(std::string(64, 'a'));
    auto address = maybe.get().data();
    auto replacement = std::string(32, 'b');
    auto source = Maybe(std::string_view(replacement));
    maybe = source;
    REQUIRE(maybe.get() == replacement);
    REQUIRE(maybe.get().data() == address);
  }

  TEST_CASE("move_assignment_in_place") {
    auto maybe = Maybe(std::string(64, 'a'));
    auto address = maybe.get().data();
    auto replacement = std::string(32, 'b');
    maybe = Maybe(std::string_view(replacement));
    REQUIRE(maybe.get() == replacement);
    REQUIRE(maybe.get().data() == address);
  }

  TEST_CASE("noexcept_assignment") {
    REQUIRE((std::is_nothrow_assignable_v<Maybe<int>&, int>));
    REQUIRE((std::is_nothrow_assignable_v<Maybe<int>&, const int&>));
    REQUIRE(!(std::is_nothrow_assignable_v<Maybe<Tracker>&, const Tracker&>));
    REQUIRE((std::is_nothrow_assignable_v<Maybe<Tracker>&, Tracker>));
  }

  TEST_CASE("const_dereferencing") {
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

  TEST_CASE("is_maybe") {
    REQUIRE(IsMaybe<Maybe<int>>);
    REQUIRE(IsMaybe<const Maybe<int>&>);
    REQUIRE(IsMaybe<Maybe<void>>);
    REQUIRE(!IsMaybe<int>);
    REQUIRE(!IsMaybe<LocalPtr<int>>);
  }

  TEST_CASE("deduction") {
    auto value = Maybe(123);
    REQUIRE((std::is_same_v<decltype(value), Maybe<int>>));
    auto copy = Maybe(value);
    REQUIRE((std::is_same_v<decltype(copy), Maybe<int>>));
    auto failure = Maybe(std::make_exception_ptr(std::runtime_error("")));
    REQUIRE((std::is_same_v<decltype(failure), Maybe<void>>));
    REQUIRE_THROWS_AS(failure.get(), std::runtime_error);
  }

  TEST_CASE("void_specialization") {
    auto maybe = Maybe<void>();
    REQUIRE(!maybe.has_exception());
    REQUIRE(!maybe.get_exception());
    maybe.get();
    auto failed = Maybe<void>(std::make_exception_ptr(std::runtime_error("")));
    REQUIRE(failed.has_exception());
    REQUIRE_THROWS_AS(failed.get(), std::runtime_error);
  }

  TEST_CASE("converting_to_a_void_specialization") {
    auto value = Maybe(123);
    auto converted = Maybe<void>(value);
    REQUIRE(!converted.has_exception());
    auto failed = Maybe<int>(std::make_exception_ptr(std::runtime_error("")));
    auto propagated = Maybe<void>(failed);
    REQUIRE(propagated.has_exception());
    REQUIRE_THROWS_AS(propagated.get(), std::runtime_error);
    auto assigned = Maybe<void>();
    assigned = failed;
    REQUIRE(assigned.has_exception());
    assigned = value;
    REQUIRE(!assigned.has_exception());
  }

  TEST_CASE("try_maybe_trait") {
    static_assert(std::is_same_v<try_maybe_t<int, true>, Maybe<int>>);
    static_assert(std::is_same_v<try_maybe_t<void, true>, Maybe<void>>);
    static_assert(std::is_same_v<try_maybe_t<void, false>, Maybe<void>>);
    static_assert(std::is_same_v<try_maybe_t<int, false>, LocalPtr<int>>);
    static_assert(
      std::is_same_v<try_maybe_t<Required, false>, std::optional<Required>>);
  }

  TEST_CASE("try_call_function") {
    static_assert(std::is_same_v<decltype(try_call([] () noexcept ->
      const int& { static const auto value = 5; return value; })),
      const int&>);
    static_assert(
      std::is_same_v<decltype(try_call([] () noexcept {})), Maybe<void>>);
    static_assert(
      std::is_same_v<decltype(try_call([] { return 5; })), Maybe<int>>);
    static_assert(std::is_same_v<decltype(try_call([] {})), Maybe<void>>);
    auto caught = try_call([] () -> int {
      throw std::runtime_error("fail");
    });
    REQUIRE(caught.has_exception());
    REQUIRE_THROWS_AS(caught.get(), std::runtime_error);
    REQUIRE(try_call([] { return 5; }).get() == 5);
  }

  TEST_CASE("converting_from_an_explicit_constructor") {
    REQUIRE((std::is_convertible_v<Maybe<int>, Maybe<long>>));
    REQUIRE(!(std::is_convertible_v<Maybe<int>, Maybe<Required>>));
    REQUIRE((std::is_constructible_v<Maybe<Required>, Maybe<int>>));
  }

  TEST_CASE("noexcept_converting_assignment") {
    static_assert(
      std::is_nothrow_assignable_v<Maybe<long>&, const Maybe<int>&>);
    static_assert(
      !std::is_nothrow_assignable_v<Maybe<Guarded>&, const Maybe<int>&>);
  }

  TEST_CASE("moving_out_a_value") {
    auto maybe = Maybe(Tracker(5));
    REQUIRE((std::is_lvalue_reference_v<decltype(maybe.get())>));
    REQUIRE((std::is_rvalue_reference_v<decltype(std::move(maybe).get())>));
    REQUIRE((std::is_rvalue_reference_v<decltype(*std::move(maybe))>));
    auto moved = std::move(maybe).get();
    REQUIRE(moved.m_value == 5);
    REQUIRE(maybe.get().m_is_moved);
  }
}

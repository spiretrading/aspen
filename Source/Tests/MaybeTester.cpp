#include <doctest/doctest.h>

import std;
import Aspen;

using namespace Aspen;

TEST_SUITE("Maybe") {
  TEST_CASE("empty_maybe") {
    auto maybe = Maybe<int>();
    REQUIRE(maybe.has_exception());
    REQUIRE(!maybe.has_value());
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
    REQUIRE(maybe.get_exception() != nullptr);
  }

  TEST_CASE("converting_maybe_exception") {
    auto maybe_int = Maybe<int>(std::make_exception_ptr(std::runtime_error("")));
    auto maybe_double = Maybe<double>(maybe_int);
    REQUIRE(maybe_double.has_exception());
  }
}

#include <catch2/catch.hpp>
#include "Aspen/Maybe.hpp"

using namespace Aspen;

TEST_CASE("test_empty_maybe", "[Maybe]") {
  auto maybe = Maybe<int>();
  REQUIRE(maybe.has_value());
  REQUIRE(!maybe.has_exception());
  REQUIRE(maybe.get() == int());
  REQUIRE(maybe == int());
}

TEST_CASE("test_value_maybe", "[Maybe]") {
  auto maybe = Maybe(123);
  REQUIRE(maybe.has_value());
  REQUIRE(!maybe.has_exception());
  REQUIRE(maybe.get() == 123);
  REQUIRE(maybe == 123);
}

TEST_CASE("test_exception_maybe", "[Maybe]") {
  auto maybe = Maybe<int>(std::make_exception_ptr(std::runtime_error("")));
  REQUIRE(!maybe.has_value());
  REQUIRE(maybe.has_exception());
  REQUIRE_THROWS_AS(maybe.get(), std::runtime_error);
  REQUIRE_THROWS_AS(maybe == 123, std::runtime_error);
  REQUIRE(maybe.get_exception() != nullptr);
}

TEST_CASE("test_converting_maybe_exception", "[Maybe]") {
  auto maybe_int = Maybe<int>(std::make_exception_ptr(std::runtime_error("")));
  auto maybe_double = Maybe<double>(maybe_int);
  REQUIRE(maybe_double.has_exception());
}

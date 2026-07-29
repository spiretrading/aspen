#include <memory>
#include <string>
#include <type_traits>
#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"

using namespace Aspen;
using namespace std::string_literals;

namespace {

  /** A value whose move constructor may throw. */
  struct Throwing {
    Throwing() = default;
    Throwing(const Throwing&) {}
    Throwing(Throwing&&) {}
  };
}

TEST_SUITE("Constant") {
  TEST_CASE("evaluating_a_constant") {
    auto integer = Constant(123);
    REQUIRE(integer.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(integer.eval() == 123);
    auto real = Constant(3.14);
    REQUIRE(real.commit(100) == State::COMPLETE_EVALUATED);
    REQUIRE(real.eval() == 3.14);
    auto text = Constant("hello world"s);
    REQUIRE(text.commit(123) == State::COMPLETE_EVALUATED);
    REQUIRE(text.eval() == "hello world"s);
  }

  TEST_CASE("wrapping_a_value") {
    auto value = 123;
    auto integer = constant(value);
    REQUIRE((std::is_same_v<decltype(integer), Constant<int>>));
    REQUIRE(integer.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(integer.eval() == 123);
    auto text = constant("hello world"s);
    REQUIRE((std::is_same_v<decltype(text), Constant<std::string>>));
    REQUIRE(text.eval() == "hello world"s);
    auto literal = constant("hello world");
    REQUIRE((std::is_same_v<decltype(literal), Constant<const char*>>));
    REQUIRE(literal.eval() == "hello world"s);
  }

  TEST_CASE("wrapping_a_move_only_value") {
    auto pointer = std::make_unique<int>(123);
    auto address = pointer.get();
    auto reactor = constant(std::move(pointer));
    REQUIRE(
      (std::is_same_v<decltype(reactor), Constant<std::unique_ptr<int>>>));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval().get() == address);
    REQUIRE(*reactor.eval() == 123);
  }

  TEST_CASE("evaluating_at_compile_time") {
    constexpr auto reactor = Constant(123);
    static_assert(reactor.eval() == 123);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("constructing_is_noexcept_when_the_move_is") {
    REQUIRE((std::is_nothrow_constructible_v<Constant<int>, int>));
    REQUIRE(
      (std::is_nothrow_constructible_v<Constant<std::string>, std::string>));
    REQUIRE(!(std::is_nothrow_constructible_v<Constant<Throwing>, Throwing>));
  }
}

#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Constant.hpp"

using namespace Aspen;

namespace {

  /** A reactor returning its evaluation by value. */
  struct ByValue {
    using Type = int;

    State commit(std::uint64_t sequence) noexcept {
      return State::COMPLETE_EVALUATED;
    }

    int eval() const noexcept {
      return 123;
    }
  };

  /** A reactor returning its evaluation by value that throws. */
  struct ThrowingByValue {
    using Type = int;

    State commit(std::uint64_t sequence) noexcept {
      return State::COMPLETE_EVALUATED;
    }

    int eval() const {
      throw std::runtime_error("fail");
    }
  };
}

TEST_SUITE("Box") {
  TEST_CASE("boxing_a_reactor") {
    auto constant = Constant(123);
    auto box = Box(std::move(constant));
    REQUIRE((std::is_same_v<decltype(box), Box<int>>));
    REQUIRE(box.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(box.eval() == 123);
    auto temporary = Box(Constant(321));
    REQUIRE(temporary.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(temporary.eval() == 321);
  }

  TEST_CASE("boxing_a_value") {
    auto reactor = box(123);
    REQUIRE((std::is_same_v<decltype(reactor), Box<int>>));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("void_box") {
    auto box = Box<void>(Constant(123));
    REQUIRE(box.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_NOTHROW(box.eval());
  }

  TEST_CASE("boxing_a_reactor_that_evaluates_by_value") {
    auto reactor = box(ByValue());
    REQUIRE((std::is_same_v<decltype(reactor), Box<int>>));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("boxing_a_reactor_of_a_convertible_type") {
    auto reactor = Box<double>(Constant(123));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    const auto& value = reactor.eval();
    REQUIRE(value == 123.0);
    REQUIRE(&value == &reactor.eval());
    REQUIRE(reactor.eval() == 123.0);
  }

  TEST_CASE("boxing_a_reactor_of_a_convertible_type_that_throws") {
    auto reactor = Box<double>(ThrowingByValue());
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("boxing_a_reactor_that_throws") {
    auto reactor = box(ThrowingByValue());
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("boxing_a_box_does_not_box_it_again") {
    auto inner = box(Constant(123));
    auto outer = box(std::move(inner));
    REQUIRE((std::is_same_v<decltype(outer), Box<int>>));
    REQUIRE(outer.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(outer.eval() == 123);
  }
}

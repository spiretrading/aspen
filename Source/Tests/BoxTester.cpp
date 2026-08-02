#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Constant.hpp"

using namespace Aspen;

namespace {
  struct ByValue {
    using Type = int;

    State commit(std::uint64_t sequence) noexcept {
      return State::COMPLETE_EVALUATED;
    }

    int eval() const noexcept {
      return 123;
    }
  };

  struct ThrowingByValue {
    using Type = int;

    State commit(std::uint64_t sequence) noexcept {
      return State::COMPLETE_EVALUATED;
    }

    int eval() const {
      throw std::runtime_error("fail");
    }
  };

  struct Checked {
    int m_value;

    Checked(int value)
        : m_value(value) {
      if(value < 0) {
        throw std::runtime_error("negative");
      }
    }
  };
}

TEST_SUITE("Box") {
  TEST_CASE("reactor") {
    auto constant = Constant(123);
    auto box = Box(std::move(constant));
    REQUIRE((std::is_same_v<decltype(box), Box<int>>));
    REQUIRE(box.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(box.eval() == 123);
    auto temporary = Box(Constant(321));
    REQUIRE(temporary.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(temporary.eval() == 321);
  }

  TEST_CASE("value") {
    auto reactor = box(123);
    REQUIRE((std::is_same_v<decltype(reactor), Box<int>>));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("void_evaluation") {
    auto box = Box<void>(Constant(123));
    REQUIRE(box.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_NOTHROW(box.eval());
  }

  TEST_CASE("by_value_reactor") {
    auto reactor = box(ByValue());
    REQUIRE((std::is_same_v<decltype(reactor), Box<int>>));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("convertible_type") {
    auto reactor = Box<double>(Constant(123));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    const auto& value = reactor.eval();
    REQUIRE(value == 123.0);
    REQUIRE(&value == &reactor.eval());
    REQUIRE(reactor.eval() == 123.0);
  }

  TEST_CASE("convertible_type_that_throws") {
    auto reactor = Box<double>(ThrowingByValue());
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("throwing_conversion_of_a_noexcept_reactor") {
    auto reactor = Box<Checked>(Constant(-1));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("exception") {
    auto reactor = box(ThrowingByValue());
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("boxing_a_box") {
    auto inner = box(Constant(123));
    auto outer = box(std::move(inner));
    REQUIRE((std::is_same_v<decltype(outer), Box<int>>));
    REQUIRE(outer.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(outer.eval() == 123);
  }
}

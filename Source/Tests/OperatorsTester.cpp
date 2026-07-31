#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Operators.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

namespace {
  template<typename T, typename U>
  void require(T&& reactor, const U& value) {
    auto state = reactor.commit(0);
    REQUIRE(has_evaluation(state));
    REQUIRE(reactor.eval() == value);
  }
}

TEST_SUITE("Operators") {
  TEST_CASE("every_operator") {
    require(constant(10) + Constant<int>(20), 30);
    require(constant(10) - Constant<int>(20), -10);
    require(constant(10) * Constant<int>(20), 200);
    require(constant(3.5) / Constant<double>(0.5), 7.0);
    require(constant(10) % Constant<int>(3), 1);
    require(constant(4) ^ Constant<int>(6), 2);
    require(Constant<int>(4) & Constant<int>(6), 4);
    require(Constant<int>(4) | Constant<int>(1), 5);
    require(~Constant<int>(4), ~4);
    require(Constant<int>(4) << constant(2), 16);
    require(Constant<int>(32) >> constant(3), 4);
    require(!Constant<bool>(true), false);
    require(!Constant<bool>(false), true);
    require(Constant<int>(5) < constant(10), true);
    require(Constant<int>(15) < constant(10), false);
    require(Constant<int>(5) <= constant(10), true);
    require(Constant<int>(5) <= constant(5), true);
    require(Constant<int>(7) <= constant(5), false);
    require(Constant<int>(5) == constant(5), true);
    require(Constant<int>(5) == constant(6), false);
    require(Constant<int>(5) != constant(5), false);
    require(Constant<int>(5) != constant(6), true);
    require(Constant<int>(12) >= constant(10), true);
    require(Constant<int>(12) >= constant(12), true);
    require(Constant<int>(7) >= constant(12), false);
    require(Constant<int>(5) > constant(10), false);
    require(Constant<int>(15) > constant(10), true);
    require(-constant(10), -10);
    require(+constant(10), +10);
    require(Constant<bool>(true) && constant(true), true);
    require(Constant<bool>(false) && constant(true), false);
    require(Constant<bool>(true) || constant(false), true);
    require(Constant<bool>(false) || constant(false), false);
  }

  TEST_CASE("plain_value") {
    require(constant(10) + 5, 15);
    require(5 + constant(10), 15);
    require(constant(10) - 5, 5);
    require(5 - constant(10), -5);
    require(constant(10) * 2, 20);
    require(constant(10) % 3, 1);
    require(constant(10) == 10, true);
    require(10 == constant(10), true);
    require(constant(10) != 10, false);
    require(constant(10) < 20, true);
    require(20 < constant(10), false);
    require(constant(10) >= 10, true);
    require(constant(4) << 2, 16);
    require(constant(32) >> 3, 4);
    require(constant(true) && false, false);
    require(false || constant(true), true);
  }

  TEST_CASE("noexcept_reactors") {
    auto left = Shared(Cell(10));
    auto right = Shared(Cell(20));
    auto reactor = left + right;
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 30);
    left->set(5);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 25);
  }

  TEST_CASE("reactor_exception") {
    auto left = Shared(Queue<int>());
    auto right = Shared(Queue<int>());
    auto reactor = left + right;
    REQUIRE(!decltype(reactor)::is_noexcept);
    left->push(1);
    right->push(2);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 3);
    left->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }
}

#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Switch.hpp"

using namespace Aspen;

TEST_SUITE("Switch") {
  TEST_CASE("empty_switch") {
    auto reactor = Switch(Constant(false), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("true_switch") {
    auto reactor = Switch(Constant(true), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
  }

  TEST_CASE("none_switch") {
    auto reactor = Switch(none<bool>(), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("flipping_switch") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = Switch(toggle, series);
    REQUIRE(reactor.commit(0) == State::NONE);
    toggle->push(true);
    REQUIRE(reactor.commit(1) == State::NONE);
    series->push(321);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 321);
    toggle->push(false);
    REQUIRE(reactor.commit(3) == State::NONE);
    REQUIRE(reactor.eval() == 321);
    toggle->push(true);
    REQUIRE(reactor.commit(4) == State::EVALUATED);
    REQUIRE(reactor.eval() == 321);
  }
}

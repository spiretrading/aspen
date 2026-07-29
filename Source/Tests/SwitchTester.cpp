#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
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

  TEST_CASE("a_toggle_that_completes_while_on") {
    auto series = Shared(Queue<int>());
    auto reactor = Switch(Constant(true), series);
    REQUIRE(reactor.commit(0) == State::NONE);
    series->push(5);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    series->push(6);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 6);
    series->set_complete();
    REQUIRE(reactor.commit(3) == State::COMPLETE);
  }

  TEST_CASE("a_toggle_that_fails_switches_off") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = Switch(toggle, series);
    series->push(1);
    toggle->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("a_toggle_that_fails_keeps_the_last_value") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = Switch(toggle, series);
    toggle->push(true);
    series->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    toggle->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(1) == State::COMPLETE);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("a_series_that_fails") {
    auto series = Shared(Queue<int>());
    auto reactor = Switch(Constant(true), series);
    REQUIRE(reactor.commit(0) == State::NONE);
    series->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("children_that_cannot_throw") {
    auto toggle = Shared(Cell(true));
    auto series = Shared(Cell(5));
    auto reactor = Switch(toggle, series);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    series->set(6);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 6);
    toggle->set(false);
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(reactor.eval() == 6);
  }

  TEST_CASE("the_switch_function") {
    auto reactor = switch_(Constant(true), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
  }
}

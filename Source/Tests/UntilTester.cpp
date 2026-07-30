#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Until.hpp"

using namespace Aspen;

TEST_SUITE("Until") {
  TEST_CASE("until_none") {
    auto reactor = Until(Constant(true), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("a_condition_reached_later") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = until(condition, series);
    condition->push(false);
    series->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    series->push(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    condition->push(true);
    REQUIRE(reactor.commit(2) == State::COMPLETE);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("a_series_completing_first") {
    auto condition = Shared(Queue<bool>());
    auto reactor = until(condition, Constant(5));
    condition->push(false);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
  }

  TEST_CASE("a_condition_that_fails_keeps_the_last_value") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = until(condition, series);
    condition->push(false);
    series->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    condition->set_complete(std::runtime_error("bad"));
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("a_series_that_fails") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = until(condition, series);
    condition->push(false);
    series->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("children_that_cannot_throw") {
    auto condition = Shared(Cell(false));
    auto series = Shared(Cell(7));
    auto reactor = until(condition, series);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 7);
    series->set(8);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 8);
    condition->set(true);
    REQUIRE(reactor.commit(2) == State::COMPLETE);
    REQUIRE(reactor.eval() == 8);
  }

  TEST_CASE("only_the_series_decides_whether_evaluating_can_throw") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Cell(5));
    auto reactor = until(condition, series);
    REQUIRE(decltype(reactor)::is_noexcept);
    condition->push(false);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
  }
}

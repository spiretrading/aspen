#include <utility>
#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Tests/ReactorTests.hpp"
#include "Aspen/When.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

TEST_SUITE("When") {
  TEST_CASE("a_condition_already_true") {
    auto reactor = when(Constant(true), Constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
  }

  TEST_CASE("a_condition_already_false") {
    auto reactor = when(Constant(false), Constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("a_condition_turning_true_later") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = when(condition, series);
    REQUIRE(reactor.commit(0) == State::NONE);
    condition->push(false);
    REQUIRE(reactor.commit(1) == State::NONE);
    series->push(1);
    REQUIRE(reactor.commit(2) == State::NONE);
    condition->push(true);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    series->push(2);
    REQUIRE(reactor.commit(4) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("a_condition_that_fails") {
    auto condition = Shared(Queue<bool>());
    auto reactor = when(condition, Constant(5));
    condition->set_complete(std::runtime_error("bad"));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("a_series_that_fails") {
    auto series = Shared(Queue<int>());
    auto reactor = when(Constant(true), series);
    series->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("children_that_cannot_throw") {
    auto condition = Shared(Cell(false));
    auto series = Shared(Cell(9));
    auto reactor = when(condition, series);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::NONE);
    condition->set(true);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 9);
    series->set(10);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
  }

  TEST_CASE("only_the_series_decides_whether_evaluating_can_throw") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Cell(9));
    auto reactor = when(condition, series);
    REQUIRE(decltype(reactor)::is_noexcept);
    condition->push(true);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 9);
  }

  TEST_CASE("a_completed_condition_is_released") {
    auto condition = TrackedReactor<bool>(true, State::COMPLETE_EVALUATED);
    auto token = condition.get_token();
    auto reactor = when(std::move(condition), Constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(token.expired());
  }
}

#include <utility>
#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Tests/ReactorTests.hpp"
#include "Aspen/Until.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

TEST_SUITE("Until") {
  TEST_CASE("no_values") {
    auto reactor = Until(Constant(true), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("condition_reached_later") {
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
  }

  TEST_CASE("continuing_condition") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    condition->push(false);
    condition->push(false);
    series->push(5);
    auto reactor = Until(condition, series);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(reactor.commit(1) == State::NONE);
  }

  TEST_CASE("continuing_series") {
    auto series = Shared(Queue<int>());
    series->push(1);
    series->push(2);
    auto reactor = Until(Constant(false), series);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("condition_completing_unreached") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = until(condition, series);
    condition->push(false);
    condition->set_complete();
    series->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    series->push(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("series_completing_first") {
    auto condition = Shared(Queue<bool>());
    auto reactor = until(condition, Constant(5));
    condition->push(false);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
  }

  TEST_CASE("condition_exception") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = until(condition, series);
    condition->push(false);
    series->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    condition->set_complete(std::runtime_error("bad"));
    REQUIRE(reactor.commit(1) == State::NONE);
  }

  TEST_CASE("series_exception") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = until(condition, series);
    condition->push(false);
    series->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("noexcept_children") {
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
  }

  TEST_CASE("noexcept_series") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Cell(5));
    auto reactor = until(condition, series);
    REQUIRE(decltype(reactor)::is_noexcept);
    condition->push(false);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
  }
  TEST_CASE("releasing_a_completed_condition") {
    auto condition = TrackedReactor<bool>(false, State::COMPLETE_EVALUATED);
    auto token = condition.get_token();
    auto series = Shared(Queue<int>());
    auto reactor = until(std::move(condition), series);
    series->push(5);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(token.expired());
  }

  TEST_CASE("releasing_a_completed_series") {
    auto series = TrackedReactor<int>(5, State::COMPLETE_EVALUATED);
    auto token = series.get_token();
    auto condition = Shared(Queue<bool>());
    auto reactor = until(condition, std::move(series));
    condition->push(false);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(token.expired());
  }

}

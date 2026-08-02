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
  TEST_CASE("condition_that_is_true") {
    auto reactor = when(Constant(true), Constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
  }

  TEST_CASE("condition_that_is_false") {
    auto reactor = when(Constant(false), Constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("condition_turning_true") {
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

  TEST_CASE("trigger_without_a_pending_value") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = when(condition, series);
    condition->push(true);
    REQUIRE(reactor.commit(0) == State::NONE);
    series->push(7);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 7);
  }

  TEST_CASE("continuing_condition") {
    auto condition = Shared(Queue<bool>());
    condition->push(false);
    condition->push(false);
    auto reactor = when(condition, Constant(5));
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::NONE);
  }

  TEST_CASE("continuing_series") {
    auto series = Shared(Queue<int>());
    series->push(1);
    series->push(2);
    auto reactor = when(Constant(true), series);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("condition_exception") {
    auto condition = Shared(Queue<bool>());
    auto reactor = when(condition, Constant(5));
    condition->set_complete(std::runtime_error("bad"));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("series_exception") {
    auto series = Shared(Queue<int>());
    auto reactor = when(Constant(true), series);
    series->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("noexcept_children") {
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

  TEST_CASE("noexcept_series") {
    auto condition = Shared(Queue<bool>());
    auto series = Shared(Cell(9));
    auto reactor = when(condition, series);
    REQUIRE(decltype(reactor)::is_noexcept);
    condition->push(true);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 9);
  }

  TEST_CASE("by_value_series") {
    auto reactor = When(Constant(true), ByValueReactor(CountedValue(1)));
    test_by_value_evaluation(reactor, 1);
  }

  TEST_CASE("by_value_series_after_the_trigger") {
    auto condition = Shared(Queue<bool>());
    auto reactor = When(condition, ByValueReactor(CountedValue(1)));
    REQUIRE(reactor.commit(0) == State::NONE);
    condition->push(true);
    REQUIRE(has_evaluation(reactor.commit(1)));
    CountedValue::reset_counts();
    [[maybe_unused]] const auto& value = reactor.eval();
    REQUIRE(CountedValue::get_destructions() == 0);
  }

  TEST_CASE("by_reference_series") {
    auto reactor = When(Constant(true), Constant(CountedValue(1)));
    REQUIRE(has_evaluation(reactor.commit(0)));
    CountedValue::reset_counts();
    [[maybe_unused]] const auto& value = reactor.eval();
    REQUIRE(CountedValue::get_copies() == 0);
  }

  TEST_CASE("releasing_a_completed_condition") {
    auto condition = TrackedReactor<bool>(true, State::COMPLETE_EVALUATED);
    auto token = condition.get_token();
    auto reactor = when(std::move(condition), Constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(token.expired());
  }
}

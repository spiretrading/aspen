#include <stdexcept>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Range.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

namespace {
  template<typename S, typename E, typename T>
  concept CanRange = requires {
    range(std::declval<S>(), std::declval<E>(), std::declval<T>());
  };

  template<typename S, typename E>
  concept CanBoundRange = requires {
    range(std::declval<S>(), std::declval<E>());
  };
}

TEST_SUITE("Range") {
  TEST_CASE("mismatched_bounds") {
    static_assert(!CanBoundRange<int, double>);
    static_assert(CanBoundRange<double, double>);
  }

  TEST_CASE("stop_below_the_start") {
    auto reactor = range(constant(10), constant(9));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("stop_at_the_start") {
    auto reactor = range(constant(1), constant(1));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("single_value") {
    auto reactor = range(constant(1), constant(2));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("two_values") {
    auto reactor = range(constant(10), constant(12));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 11);
  }

  TEST_CASE("completing_stop") {
    auto queue = Shared(Queue<int>());
    auto reactor = range(queue, constant(10));
    queue->push(8);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 8);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 9);
  }

  TEST_CASE("changing_start") {
    auto queue = Shared(Queue<int>());
    auto reactor = range(queue, constant(100));
    queue->push(8);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 8);
    queue->push(50);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 50);
    queue->push(20);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 51);
    queue->push(200);
    REQUIRE(reactor.commit(3) == State::COMPLETE);
  }

  TEST_CASE("step") {
    auto reactor = range(constant(0), constant(10), constant(3));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 6);
    REQUIRE(reactor.commit(3) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 9);
  }

  TEST_CASE("step_that_is_a_value") {
    auto reactor = range(constant(0), constant(10), 4);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 4);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 8);
  }

  TEST_CASE("noexcept_start") {
    auto cell = Shared(Cell(0));
    auto reactor = range(cell, constant(3));
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
  }

  TEST_CASE("start_exception") {
    auto queue = Shared(Queue<int>());
    auto reactor = range(queue, constant(10));
    REQUIRE(!decltype(reactor)::is_noexcept);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("changing_stop") {
    auto start_queue = Shared(Queue<int>());
    auto end_queue = Shared(Queue<int>());
    auto reactor = range(start_queue, end_queue);
    start_queue->push(0);
    end_queue->push(10);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    start_queue->push(6);
    end_queue->push(9);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 6);
    end_queue->push(3);
    REQUIRE(reactor.commit(3) == State::NONE);
    end_queue->push(7);
    REQUIRE(reactor.commit(4) == State::NONE);
    end_queue->push(8);
    REQUIRE(reactor.commit(5) == State::EVALUATED);
    REQUIRE(reactor.eval() == 7);
    end_queue->set_complete();
    REQUIRE(reactor.commit(6) == State::COMPLETE);
  }

  TEST_CASE("step_wider_than_one") {
    static_assert(!CanRange<int, double, double>);
    static_assert(CanRange<double, double, double>);
    auto reactor = range(0.0, 2.5, 0.5);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0.5);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1.0);
    REQUIRE(reactor.commit(3) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1.5);
    REQUIRE(reactor.commit(4) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 2.0);
  }

  TEST_CASE("start_moving_ahead") {
    auto start = Shared(Cell(0));
    auto reactor = range(start, 100, 1);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    start->set(20);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 20);
    REQUIRE(reactor.commit(3) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 21);
  }

  TEST_CASE("start_moving_behind") {
    auto start = Shared(Cell(10));
    auto reactor = range(start, 100, 5);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 15);
    start->set(12);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 20);
  }

  TEST_CASE("changing_step") {
    auto step = Shared(Cell(1));
    auto reactor = range(0, 100, step);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    step->set(5);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 6);
    REQUIRE(reactor.commit(3) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 11);
  }

  TEST_CASE("step_of_zero") {
    auto step = Shared(Cell(0));
    auto reactor = range(constant(0), constant(10), step);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::NONE);
    step->set(2);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("negative_start") {
    auto reactor = range(-2, 2);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == -2);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == -1);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(3) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("stop_exception") {
    auto queue = Shared(Queue<int>());
    auto reactor = range(constant(0), queue);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("step_exception") {
    auto queue = Shared(Queue<int>());
    auto reactor = range(constant(0), constant(10), queue);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("negative_step") {
    auto reactor = range(constant(0), constant(10), constant(-1));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }
}

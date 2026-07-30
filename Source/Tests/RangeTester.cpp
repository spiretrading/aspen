#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Range.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Range") {
  TEST_CASE("backward_range") {
    auto reactor = range(constant(10), constant(9));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("empty_range") {
    auto reactor = range(constant(1), constant(1));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("single_range") {
    auto reactor = range(constant(1), constant(2));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("double_range") {
    auto reactor = range(constant(10), constant(12));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 11);
  }

  TEST_CASE("end_complete") {
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
    REQUIRE(reactor.eval() == 51);
  }

  TEST_CASE("range_with_a_step") {
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

  TEST_CASE("range_with_a_step_that_is_a_value") {
    auto reactor = range(constant(0), constant(10), 4);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 4);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 8);
  }

  TEST_CASE("a_start_that_cannot_throw") {
    auto cell = Shared(Cell(0));
    auto reactor = range(cell, constant(3));
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
  }

  TEST_CASE("a_start_that_fails") {
    auto queue = Shared(Queue<int>());
    auto reactor = range(queue, constant(10));
    REQUIRE(!decltype(reactor)::is_noexcept);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("changing_end") {
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
  TEST_CASE("a_step_wider_than_the_start") {
    auto reactor = range(0, 2.5, 0.5);
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

}

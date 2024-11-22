#include <doctest/doctest.h>

import <stdexcept>;
import Aspen;

using namespace Aspen;

TEST_SUITE("Queue") {
  TEST_CASE("queue_immediate_complete") {
    auto queue = Queue<int>();
    queue.set_complete();
    REQUIRE(queue.commit(0) == State::COMPLETE);
  }

  TEST_CASE("queue_complete_with_exception") {
    auto queue = Queue<int>();
    queue.set_complete(std::runtime_error(""));
    REQUIRE(queue.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  }

  TEST_CASE("queue_single_value") {
    auto queue = Queue<int>();
    queue.set_complete(123);
    REQUIRE(queue.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(queue.eval() == 123);
  }

  TEST_CASE("queue_single_value_then_complete") {
    auto queue = Queue<int>();
    queue.push(321);
    REQUIRE(queue.commit(0) == State::EVALUATED);
    REQUIRE(queue.eval() == 321);
    queue.set_complete();
    REQUIRE(queue.commit(1) == State::COMPLETE);
    REQUIRE(queue.eval() == 321);
  }

  TEST_CASE("queue_single_value_then_exception") {
    auto queue = Queue<int>();
    queue.push(321);
    REQUIRE(queue.commit(0) == State::EVALUATED);
    REQUIRE(queue.eval() == 321);
    queue.set_complete(std::runtime_error(""));
    REQUIRE(queue.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  }

  TEST_CASE("queue_empty_then_complete") {
    auto queue = Queue<int>();
    REQUIRE(queue.commit(0) == State::NONE);
    queue.set_complete();
    REQUIRE(queue.commit(1) == State::COMPLETE);
  }

  TEST_CASE("queue_empty_then_evaluated") {
    auto queue = Queue<int>();
    REQUIRE(queue.commit(0) == State::NONE);
    queue.push(1);
    REQUIRE(queue.commit(1) == State::EVALUATED);
    REQUIRE(queue.eval() == 1);
  }

  TEST_CASE("queue_empty_then_complete_evaluated") {
    auto queue = Queue<int>();
    REQUIRE(queue.commit(0) == State::NONE);
    queue.set_complete(1);
    REQUIRE(queue.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(queue.eval() == 1);
  }

  TEST_CASE("queue_empty_then_complete_exception") {
    auto queue = Queue<int>();
    REQUIRE(queue.commit(0) == State::NONE);
    queue.set_complete(std::runtime_error("fail"));
    REQUIRE(queue.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  }
}

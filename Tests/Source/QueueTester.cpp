#include <exception>
#include <catch2/catch.hpp>
#include "Aspen/Queue.hpp"

using namespace Aspen;

TEST_CASE("test_queue_immediate_complete", "[Queue]") {
  auto queue = Queue<int>();
  queue.set_complete();
  REQUIRE(queue.commit(0) == State::COMPLETE_EMPTY);
  REQUIRE(queue.commit(0) == State::COMPLETE_EMPTY);
  REQUIRE(queue.commit(100) == State::COMPLETE_EMPTY);
  REQUIRE(queue.commit(0) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_queue_complete_with_exception", "[Queue]") {
  auto queue = Queue<int>();
  queue.set_complete(std::runtime_error(""));
  REQUIRE(queue.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  REQUIRE(queue.commit(1) == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  REQUIRE(queue.commit(100) == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  REQUIRE(queue.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
}

TEST_CASE("test_queue_single_value", "[Queue]") {
  auto queue = Queue<int>();
  queue.set_complete(123);
  REQUIRE(queue.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(queue.eval() == 123);
  REQUIRE(queue.commit(1) == State::COMPLETE_EVALUATED);
  REQUIRE(queue.eval() == 123);
  REQUIRE(queue.commit(100) == State::COMPLETE_EVALUATED);
  REQUIRE(queue.eval() == 123);
  REQUIRE(queue.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(queue.eval() == 123);
}

TEST_CASE("test_queue_single_value_then_complete", "[Queue]") {
  auto queue = Queue<int>();
  queue.push(321);
  REQUIRE(queue.commit(0) == State::EVALUATED);
  REQUIRE(queue.eval() == 321);
  queue.set_complete();
  REQUIRE(queue.commit(1) == State::COMPLETE);
  REQUIRE(queue.eval() == 321);
}

TEST_CASE("test_queue_single_value_then_exception", "[Queue]") {
  auto queue = Queue<int>();
  queue.push(321);
  REQUIRE(queue.commit(0) == State::EVALUATED);
  REQUIRE(queue.eval() == 321);
  queue.set_complete(std::runtime_error(""));
  REQUIRE(queue.commit(1) == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
}

TEST_CASE("test_queue_empty_then_complete", "[Queue]") {
  auto queue = Queue<int>();
  REQUIRE(queue.commit(0) == State::EMPTY);
  queue.set_complete();
  REQUIRE(queue.commit(1) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_queue_empty_then_evaluated", "[Queue]") {
  auto queue = Queue<int>();
  REQUIRE(queue.commit(0) == State::EMPTY);
  queue.push(1);
  REQUIRE(queue.commit(1) == State::EVALUATED);
  REQUIRE(queue.eval() == 1);
}

TEST_CASE("test_queue_empty_then_complete_evaluated", "[Queue]") {
  auto queue = Queue<int>();
  REQUIRE(queue.commit(0) == State::EMPTY);
  queue.set_complete(1);
  REQUIRE(queue.commit(1) == State::COMPLETE_EVALUATED);
  REQUIRE(queue.eval() == 1);
}

TEST_CASE("test_queue_empty_then_complete_exception", "[Queue]") {
  auto queue = Queue<int>();
  REQUIRE(queue.commit(0) == State::EMPTY);
  queue.set_complete(std::runtime_error("fail"));
  REQUIRE(queue.commit(1) == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
}

#include <exception>
#include <catch2/catch.hpp>
#include "Aspen/Queue.hpp"

using namespace Aspen;

TEST_CASE("test_queue_immediate_complete", "[Queue]") {
  auto queue = Queue<int>();
  queue.set_complete();
  auto state = queue.commit(0);
  REQUIRE(state == State::COMPLETE_EMPTY);
  state = queue.commit(0);
  REQUIRE(state == State::COMPLETE_EMPTY);
  state = queue.commit(100);
  REQUIRE(state == State::COMPLETE_EMPTY);
  state = queue.commit(0);
  REQUIRE(state == State::COMPLETE_EMPTY);
  Trigger::set_trigger(nullptr);
}

TEST_CASE("test_queue_complete_with_exception", "[Queue]") {
  auto queue = Queue<int>();
  queue.set_complete(std::runtime_error(""));
  auto state = queue.commit(0);
  REQUIRE(state == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  state = queue.commit(1);
  REQUIRE(state == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  state = queue.commit(100);
  REQUIRE(state == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  state = queue.commit(0);
  REQUIRE(state == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
}

TEST_CASE("test_queue_single_value", "[Queue]") {
  auto queue = Queue<int>();
  queue.set_complete(123);
  auto state = queue.commit(0);
  REQUIRE(state == State::COMPLETE_EVALUATED);
  REQUIRE(queue.eval() == 123);
  state = queue.commit(1);
  REQUIRE(state == State::COMPLETE_EVALUATED);
  REQUIRE(queue.eval() == 123);
  state = queue.commit(100);
  REQUIRE(state == State::COMPLETE_EVALUATED);
  REQUIRE(queue.eval() == 123);
  state = queue.commit(0);
  REQUIRE(state == State::COMPLETE_EVALUATED);
  REQUIRE(queue.eval() == 123);
}

TEST_CASE("test_queue_single_value_then_complete", "[Queue]") {
  auto queue = Queue<int>();
  queue.push(321);
  auto state = queue.commit(0);
  REQUIRE(state == State::EVALUATED);
  REQUIRE(queue.eval() == 321);
  queue.set_complete();
  state = queue.commit(1);
  REQUIRE(state == State::COMPLETE);
  REQUIRE(queue.eval() == 321);
}

TEST_CASE("test_queue_single_value_then_exception", "[Queue]") {
  auto queue = Queue<int>();
  queue.push(321);
  auto state = queue.commit(0);
  REQUIRE(state == State::EVALUATED);
  REQUIRE(queue.eval() == 321);
  queue.set_complete(std::runtime_error(""));
  state = queue.commit(1);
  REQUIRE(state == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
}

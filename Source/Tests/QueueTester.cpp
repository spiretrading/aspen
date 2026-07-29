#include <exception>
#include <stdexcept>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Trigger.hpp"

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

  TEST_CASE("queue_several_values") {
    auto queue = Queue<int>();
    queue.push(1);
    queue.push(2);
    queue.push(3);
    REQUIRE(queue.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(queue.eval() == 1);
    REQUIRE(queue.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(queue.eval() == 2);
    REQUIRE(queue.commit(2) == State::EVALUATED);
    REQUIRE(queue.eval() == 3);
    REQUIRE(queue.commit(3) == State::NONE);
  }

  TEST_CASE("queue_move_construction") {
    auto queue = Queue<int>();
    queue.push(1);
    queue.push(2);
    auto moved = Queue<int>(std::move(queue));
    REQUIRE(moved.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(moved.eval() == 1);
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved.eval() == 2);
  }

  TEST_CASE("queue_move_assignment") {
    auto queue = Queue<int>();
    queue.push(1);
    queue.set_complete();
    auto moved = Queue<int>();
    moved.push(99);
    moved = std::move(queue);
    REQUIRE(moved.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(moved.eval() == 1);
  }

  TEST_CASE("queue_self_move_assignment") {
    auto queue = Queue<int>();
    queue.push(1);
    auto& alias = queue;
    queue = std::move(alias);
    REQUIRE(queue.commit(0) == State::EVALUATED);
    REQUIRE(queue.eval() == 1);
  }

  TEST_CASE("pushing_from_the_slot_a_push_signals") {
    auto queue = Queue<int>();
    auto flag = CommitFlag();
    auto is_pushed = false;
    auto trigger = Trigger([&] {
      if(!is_pushed) {
        is_pushed = true;
        queue.push(2);
      }
    });
    flag.set_trigger(&trigger);
    {
      auto scope = CommitFlagScope(flag);
      REQUIRE(queue.commit(0) == State::NONE);
    }
    flag.clear();
    queue.push(1);
    REQUIRE(is_pushed);
    REQUIRE(queue.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(queue.eval() == 1);
    REQUIRE(queue.commit(2) == State::EVALUATED);
    REQUIRE(queue.eval() == 2);
  }
}

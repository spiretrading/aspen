#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Trigger.hpp"

using namespace Aspen;

TEST_SUITE("Queue") {
  TEST_CASE("immediate_completion") {
    auto queue = Queue<int>();
    queue.set_complete();
    REQUIRE(queue.commit(0) == State::COMPLETE);
  }

  TEST_CASE("immediate_exception") {
    auto queue = Queue<int>();
    queue.set_complete(std::runtime_error(""));
    REQUIRE(queue.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  }

  TEST_CASE("value") {
    auto queue = Queue<int>();
    queue.set_complete(123);
    REQUIRE(queue.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(queue.eval() == 123);
  }

  TEST_CASE("value_then_completion") {
    auto queue = Queue<int>();
    queue.push(321);
    REQUIRE(queue.commit(0) == State::EVALUATED);
    REQUIRE(queue.eval() == 321);
    queue.set_complete();
    REQUIRE(queue.commit(1) == State::COMPLETE);
  }

  TEST_CASE("value_then_an_exception") {
    auto queue = Queue<int>();
    queue.push(321);
    REQUIRE(queue.commit(0) == State::EVALUATED);
    REQUIRE(queue.eval() == 321);
    queue.set_complete(std::runtime_error(""));
    REQUIRE(queue.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  }

  TEST_CASE("completion_while_empty") {
    auto queue = Queue<int>();
    REQUIRE(queue.commit(0) == State::NONE);
    queue.set_complete();
    REQUIRE(queue.commit(1) == State::COMPLETE);
  }

  TEST_CASE("value_while_empty") {
    auto queue = Queue<int>();
    REQUIRE(queue.commit(0) == State::NONE);
    queue.push(1);
    REQUIRE(queue.commit(1) == State::EVALUATED);
    REQUIRE(queue.eval() == 1);
  }

  TEST_CASE("completion_with_a_value_while_empty") {
    auto queue = Queue<int>();
    REQUIRE(queue.commit(0) == State::NONE);
    queue.set_complete(1);
    REQUIRE(queue.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(queue.eval() == 1);
  }

  TEST_CASE("exception_while_empty") {
    auto queue = Queue<int>();
    REQUIRE(queue.commit(0) == State::NONE);
    queue.set_complete(std::runtime_error("fail"));
    REQUIRE(queue.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  }

  TEST_CASE("several_values") {
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

  TEST_CASE("move_construction") {
    auto queue = Queue<int>();
    queue.push(1);
    queue.push(2);
    auto moved = Queue<int>(std::move(queue));
    REQUIRE(moved.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(moved.eval() == 1);
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved.eval() == 2);
  }

  TEST_CASE("move_construction_after_a_commit") {
    auto queue = Queue<int>();
    queue.push(5);
    REQUIRE(queue.commit(0) == State::EVALUATED);
    REQUIRE(queue.eval() == 5);
    auto moved = Queue<int>(std::move(queue));
    REQUIRE(moved.commit(0) == State::EVALUATED);
    REQUIRE(moved.eval() == 5);
    moved.push(6);
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved.eval() == 6);
  }

  TEST_CASE("move_assignment") {
    auto queue = Queue<int>();
    queue.push(1);
    queue.set_complete();
    auto moved = Queue<int>();
    moved.push(99);
    moved = std::move(queue);
    REQUIRE(moved.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(moved.eval() == 1);
  }

  TEST_CASE("self_move_assignment") {
    auto queue = Queue<int>();
    queue.push(1);
    auto& alias = queue;
    queue = std::move(alias);
    REQUIRE(queue.commit(0) == State::EVALUATED);
    REQUIRE(queue.eval() == 1);
  }

  TEST_CASE("pushing_from_a_slot") {
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

  TEST_CASE("completing_with_a_convertible_value") {
    auto queue = Queue<std::string>();
    queue.set_complete("done");
    REQUIRE(queue.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(queue.eval() == "done");
  }

  TEST_CASE("completing_with_a_widening_value") {
    auto queue = Queue<long>();
    queue.set_complete(3);
    REQUIRE(queue.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(queue.eval() == 3);
  }

  TEST_CASE("completing_with_a_null_exception") {
    auto queue = Queue<int>();
    queue.set_complete(std::exception_ptr());
    REQUIRE(queue.commit(0) == State::COMPLETE);
  }

  TEST_CASE("assignment_after_an_evaluation") {
    auto queue = Queue<int>();
    queue.push(7);
    REQUIRE(queue.commit(0) == State::EVALUATED);
    REQUIRE(queue.eval() == 7);
    auto source = Queue<int>();
    source.push(9);
    queue = std::move(source);
    REQUIRE(queue.eval() == 7);
    REQUIRE(queue.commit(1) == State::EVALUATED);
    REQUIRE(queue.eval() == 9);
  }

  TEST_CASE("pushing_after_completion") {
    auto queue = Queue<int>();
    queue.set_complete(std::runtime_error("fail"));
    REQUIRE(queue.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
    queue.push(5);
    REQUIRE_THROWS_AS(queue.eval(), std::runtime_error);
  }

  TEST_CASE("raising_after_completion") {
    auto queue = Queue<int>();
    auto flag = CommitFlag();
    {
      auto scope = CommitFlagScope(flag);
      queue.set_complete();
      REQUIRE(queue.commit(0) == State::COMPLETE);
    }
    flag.clear();
    queue.push(5);
    REQUIRE(!flag.is_raised());
  }

  TEST_CASE("move_assignment_after_a_commit") {
    auto flag = CommitFlag();
    auto destination = Queue<int>();
    {
      auto scope = CommitFlagScope(flag);
      REQUIRE(destination.commit(0) == State::NONE);
    }
    flag.clear();
    auto source = Queue<int>();
    source.push(5);
    REQUIRE(source.commit(0) == State::EVALUATED);
    REQUIRE(source.eval() == 5);
    destination = std::move(source);
    REQUIRE(flag.is_raised());
    REQUIRE(destination.commit(1) == State::EVALUATED);
    REQUIRE(destination.eval() == 5);
  }

  TEST_CASE("move_assignment_in_a_graph") {
    auto flag = CommitFlag();
    auto destination = Queue<int>();
    {
      auto scope = CommitFlagScope(flag);
      REQUIRE(destination.commit(0) == State::NONE);
    }
    flag.clear();
    auto source = Queue<int>();
    source.push(5);
    destination = std::move(source);
    REQUIRE(flag.is_raised());
    flag.clear();
    destination.push(10);
    REQUIRE(flag.is_raised());
    REQUIRE(destination.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(destination.eval() == 5);
    REQUIRE(destination.commit(2) == State::EVALUATED);
    REQUIRE(destination.eval() == 10);
  }
}

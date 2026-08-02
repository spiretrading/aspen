#include <optional>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Cell.hpp"
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Weak.hpp"

using namespace Aspen;

TEST_SUITE("Weak") {
  TEST_CASE("queue") {
    auto s1 = std::optional<Shared<Queue<int>>>();
    s1.emplace();
    auto s2 = Weak(*s1);
    (*s1)->push(5);
    (*s1)->push(10);
    (*s1)->push(15);
    REQUIRE(s2.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(s2.eval() == 5);
    REQUIRE(s2.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(s2.eval() == 10);
    s1 = std::nullopt;
    REQUIRE(s2.commit(2) == State::COMPLETE);
  }

  TEST_CASE("box") {
    auto s1 = std::optional<Shared<Queue<int>>>();
    s1.emplace();
    auto s2 = std::optional<Shared<Box<int>>>(*s1);
    auto s3 = Weak(*s2);
    (*s1)->push(5);
    (*s1)->push(10);
    (*s1)->push(15);
    REQUIRE(s3.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(s3.eval() == 5);
    REQUIRE(s3.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(s3.eval() == 10);
    s1 = std::nullopt;
    s2 = std::nullopt;
    REQUIRE(s3.commit(2) == State::COMPLETE);
  }

  TEST_CASE("noexcept_reactor") {
    auto shared = std::optional(Shared(Cell(1)));
    auto weak = Weak(*shared);
    REQUIRE(decltype(weak)::is_noexcept);
    REQUIRE(weak.commit(0) == State::EVALUATED);
    REQUIRE(weak.eval() == 1);
    (*shared)->set(2);
    REQUIRE(weak.commit(1) == State::EVALUATED);
    REQUIRE(weak.eval() == 2);
    shared = std::nullopt;
    REQUIRE(weak.commit(2) == State::COMPLETE);
  }

  TEST_CASE("locking") {
    auto shared = std::optional(Shared(Queue<int>()));
    auto weak = Weak(*shared);
    auto locked = weak.lock();
    REQUIRE(locked);
    (*locked)->push(5);
    REQUIRE(weak.commit(0) == State::EVALUATED);
    REQUIRE(weak.eval() == 5);
    locked = std::nullopt;
    shared = std::nullopt;
    REQUIRE(!weak.lock());
  }

  TEST_CASE("copy_and_move_construction") {
    auto shared = std::optional(Shared(Queue<int>()));
    auto weak = Weak(*shared);
    (*shared)->push(5);
    auto copy = weak;
    REQUIRE(copy.commit(0) == State::EVALUATED);
    REQUIRE(copy.eval() == 5);
    auto moved = std::move(copy);
    (*shared)->push(10);
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved.eval() == 10);
    REQUIRE(moved.lock());
  }

  TEST_CASE("moving_the_observed_shared") {
    auto shared = std::optional(Shared(Queue<int>()));
    (*shared)->push(5);
    auto weak = Weak(*shared);
    REQUIRE(weak.commit(0) == State::EVALUATED);
    REQUIRE(weak.eval() == 5);
    auto moved = std::optional(std::move(*shared));
    shared = std::nullopt;
    (*moved)->push(10);
    REQUIRE(weak.commit(1) == State::EVALUATED);
    REQUIRE(weak.eval() == 10);
    moved = std::nullopt;
    REQUIRE(weak.commit(2) == State::COMPLETE);
  }

  TEST_CASE("completing_after_a_consumed_evaluation") {
    auto cell = Shared(Cell(1));
    auto boxed = std::optional(Shared<Box<double>>(cell));
    auto weak = Weak(*boxed);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    REQUIRE(boxed->commit(0) == State::EVALUATED);
    REQUIRE(weak.commit(0) == State::EVALUATED);
    cell->set(2);
    REQUIRE(cell.commit(1) == State::EVALUATED);
    REQUIRE(boxed->commit(2) == State::EVALUATED);
    REQUIRE(weak.commit(2) == State::EVALUATED);
    REQUIRE(weak.eval() == 2);
    boxed = std::nullopt;
    cell->set(3);
    REQUIRE(weak.commit(3) == State::COMPLETE);
  }

  TEST_CASE("copy_assigning_the_shared") {
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    auto weak = Weak(first);
    first->push(5);
    REQUIRE(weak.commit(0) == State::EVALUATED);
    REQUIRE(weak.eval() == 5);
    first = second;
    REQUIRE(weak.commit(1) == State::COMPLETE);
  }

  TEST_CASE("move_assigning_the_shared") {
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    auto weak = Weak(first);
    first->push(5);
    REQUIRE(weak.commit(0) == State::EVALUATED);
    REQUIRE(weak.eval() == 5);
    first = std::move(second);
    REQUIRE(weak.commit(1) == State::COMPLETE);
  }

  TEST_CASE("assignment") {
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    first->push(1);
    second->push(2);
    auto weak = Weak(first);
    REQUIRE(weak.commit(0) == State::EVALUATED);
    REQUIRE(weak.eval() == 1);
    weak = Weak(second);
    REQUIRE(weak.commit(1) == State::EVALUATED);
    REQUIRE(weak.eval() == 2);
    auto moved = Weak(first);
    weak = std::move(moved);
    REQUIRE(weak.commit(2) == State::EVALUATED);
    REQUIRE(weak.eval() == 1);
  }

  TEST_CASE("uncommitted_box") {
    auto cell = Shared(Cell(1));
    auto boxed = std::optional(Shared<Box<double>>(cell));
    auto observer = Weak(*boxed);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    boxed = std::nullopt;
    REQUIRE(observer.commit(1) == State::COMPLETE);
  }

  TEST_CASE("flag_after_a_move") {
    auto cell = Shared(Cell(1));
    auto observer = Weak(cell);
    auto flag = CommitFlag();
    {
      auto scope = CommitFlagScope(flag);
      REQUIRE(observer.commit(0) == State::EVALUATED);
    }
    auto moved = std::move(observer);
    flag.clear();
    cell->set(2);
    REQUIRE(!flag.is_raised());
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved.eval() == 2);
  }

  TEST_CASE("flag_after_a_move_assignment") {
    auto cell = Shared(Cell(1));
    auto other = Shared(Cell(0));
    auto observer = Weak(cell);
    auto flag = CommitFlag();
    {
      auto scope = CommitFlagScope(flag);
      REQUIRE(observer.commit(0) == State::EVALUATED);
    }
    auto moved = Weak(other);
    moved = std::move(observer);
    flag.clear();
    cell->set(2);
    REQUIRE(!flag.is_raised());
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved.eval() == 2);
  }
}

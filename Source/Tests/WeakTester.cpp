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
  TEST_CASE("weak_queue") {
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
    REQUIRE(s2.eval() == 10);
  }

  TEST_CASE("weak_box") {
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
    REQUIRE(s3.eval() == 10);
  }

  TEST_CASE("a_reactor_that_cannot_throw") {
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
    REQUIRE(weak.eval() == 2);
  }

  TEST_CASE("locking_a_weak") {
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

  TEST_CASE("copying_and_moving_a_weak") {
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
  TEST_CASE("weak_from_a_moved_shared") {
    auto shared = std::optional(Shared(Queue<int>()));
    (*shared)->push(5);
    REQUIRE(shared->commit(0) == State::EVALUATED);
    REQUIRE(shared->eval() == 5);
    auto weak = Weak(std::move(*shared));
    shared = std::nullopt;
    REQUIRE(weak.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(weak.eval() == 5);
  }

  TEST_CASE("copy_assigning_an_observed_shared") {
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    auto weak = Weak(first);
    first->push(5);
    REQUIRE(weak.commit(0) == State::EVALUATED);
    REQUIRE(weak.eval() == 5);
    first = second;
    REQUIRE(weak.commit(1) == State::COMPLETE);
    REQUIRE(weak.eval() == 5);
  }

  TEST_CASE("move_assigning_an_observed_shared") {
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    auto weak = Weak(first);
    first->push(5);
    REQUIRE(weak.commit(0) == State::EVALUATED);
    REQUIRE(weak.eval() == 5);
    first = std::move(second);
    REQUIRE(weak.commit(1) == State::COMPLETE);
    REQUIRE(weak.eval() == 5);
  }

  TEST_CASE("assigning_a_weak") {
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

  TEST_CASE("a_weak_over_an_uncommitted_box_reports_no_evaluation") {
    auto cell = Shared(Cell(1));
    auto boxed = std::optional(Shared<Box<double>>(cell));
    auto observer = Weak(*boxed);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    boxed = std::nullopt;
    REQUIRE(observer.commit(1) == State::COMPLETE);
  }

  TEST_CASE("moving_a_weak_stops_reporting_to_the_old_flag") {
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

  TEST_CASE("move_assigning_a_weak_stops_reporting_to_the_old_flag") {
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

#include <optional>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Cell.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/StateReactor.hpp"
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

TEST_SUITE("Shared") {
  TEST_CASE("chain") {
    auto s1 = Shared(Chain(constant(10), constant(9)));
    auto s2 = s1;
    REQUIRE(s1.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(s1.eval() == 10);
    REQUIRE(s2.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(s2.eval() == 10);
    REQUIRE(s1.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(s1.eval() == 9);
    REQUIRE(s2.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(s2.eval() == 9);
  }

  TEST_CASE("from_a_unique") {
    auto c = Unique(std::make_unique<Constant<int>>(5));
    auto s = Shared(std::move(c));
    REQUIRE(s.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(s.eval() == 5);
  }

  TEST_CASE("to_a_box") {
    auto c = Shared(Constant(123));
    auto b = Shared<Box<int>>(c);
    REQUIRE(b.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(b.eval() == 123);
  }

  TEST_CASE("move_construction") {
    auto queue = Shared(Queue<int>());
    queue->push(1);
    auto moved = Shared(std::move(queue));
    REQUIRE(moved.commit(0) == State::EVALUATED);
    REQUIRE(moved.eval() == 1);
    moved->push(2);
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved.eval() == 2);
  }

  TEST_CASE("copy_assignment") {
    auto left = Shared(Queue<int>());
    auto right = Shared(Queue<int>());
    left->push(1);
    right->push(2);
    REQUIRE(left.commit(0) == State::EVALUATED);
    REQUIRE(left.eval() == 1);
    left = right;
    REQUIRE(left.commit(1) == State::EVALUATED);
    REQUIRE(left.eval() == 2);
    right->push(3);
    REQUIRE(left.commit(2) == State::EVALUATED);
    REQUIRE(left.eval() == 3);
  }

  TEST_CASE("move_assignment") {
    auto left = Shared(Queue<int>());
    auto right = Shared(Queue<int>());
    left->push(1);
    right->push(2);
    REQUIRE(left.commit(0) == State::EVALUATED);
    REQUIRE(left.eval() == 1);
    left = std::move(right);
    REQUIRE(left.commit(1) == State::EVALUATED);
    REQUIRE(left.eval() == 2);
  }

  TEST_CASE("self_assignment") {
    auto queue = Shared(Queue<int>());
    queue->push(1);
    REQUIRE(queue.commit(0) == State::EVALUATED);
    REQUIRE(queue.eval() == 1);
    auto& alias = queue;
    queue = alias;
    queue = std::move(alias);
    queue->push(2);
    REQUIRE(queue.commit(1) == State::EVALUATED);
    REQUIRE(queue.eval() == 2);
  }

  TEST_CASE("noexcept_reactor") {
    auto cell = Shared(Cell(1));
    REQUIRE(decltype(cell)::is_noexcept);
    auto queue = Shared(Queue<int>());
    REQUIRE(!decltype(queue)::is_noexcept);
    static_assert(IsReactorOf<Shared<Cell<int>>, int>);
    static_assert(std::is_same_v<collapse_shared_t<Shared<Shared<Cell<int>>>>,
      Shared<Cell<int>>>);
  }

  TEST_CASE("emplaced_construction") {
    auto reactor = Shared<Cell<int>>(std::in_place, 7);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 7);
  }

  TEST_CASE("none") {
    auto a = Shared(chain(123, none<int>(), 321, none<int>()));
    auto b = Shared(a);
    a.commit(0);
    REQUIRE(b.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(b.eval() == 123);
    a.commit(1);
    a.commit(2);
    a.commit(3);
    REQUIRE(b.commit(3) == State::COMPLETE_EVALUATED);
    REQUIRE(b.eval() == 321);
  }

  TEST_CASE("destroying_unobserved") {
    auto reactor = std::optional(Shared(Constant(CountedValue(1))));
    REQUIRE(has_evaluation(reactor->commit(0)));
    CountedValue::reset_counts();
    reactor = std::nullopt;
    REQUIRE(CountedValue::get_copies() == 0);
  }

  TEST_CASE("boxed_evaluation_cache") {
    auto reactor = Shared(ByValueReactor(5));
    auto first = shared_box(reactor);
    auto second = shared_box(reactor);
    REQUIRE(first.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(first.eval() == 5);
    REQUIRE(second.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(second.eval() == 5);
  }

  TEST_CASE("boxed_commit") {
    auto cell = Shared(Cell(1));
    auto first = Shared<Box<double>>(cell);
    auto second = Shared<Box<double>>(cell);
    REQUIRE(first.commit(0) == State::EVALUATED);
    REQUIRE(first.eval() == 1.0);
    REQUIRE(second.commit(0) == State::EVALUATED);
    REQUIRE(second.eval() == 1.0);
    cell->set(2);
    REQUIRE(first.commit(1) == State::EVALUATED);
    REQUIRE(first.eval() == 2.0);
    REQUIRE(second.commit(1) == State::EVALUATED);
    REQUIRE(second.eval() == 2.0);
  }

  TEST_CASE("skipping_a_quiet_sequence") {
    auto reactor = Shared(CountingReactor<int>(1, State::EVALUATED));
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor->get_commits() == 1);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(reactor->get_commits() == 1);
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(reactor->get_commits() == 1);
  }

  TEST_CASE("sharing_state_with_a_box") {
    static_assert(IsBox<Box<int>>);
    static_assert(!IsBox<Cell<int>>);
    static_assert(std::is_convertible_v<Shared<Cell<int>>, Shared<Box<int>>>);
    static_assert(!std::is_convertible_v<Shared<Cell<int>>,
      Shared<StateReactor<Shared<Cell<int>>>>>);
    static_assert(std::is_constructible_v<
      Shared<StateReactor<Shared<Cell<int>>>>, Shared<Cell<int>>>);
    auto cell = Shared(Cell(1));
    auto states = Shared<StateReactor<Shared<Cell<int>>>>(cell);
    REQUIRE(states.commit(0) == State::EVALUATED);
    REQUIRE(states.eval() == State::EVALUATED);
    cell->set(2);
    REQUIRE(states.commit(1) == State::EVALUATED);
    REQUIRE(states.eval() == State::EVALUATED);
  }

  TEST_CASE("flag_after_a_move") {
    auto cell = Shared(Cell(1));
    auto observer = cell;
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
    auto observer = cell;
    auto flag = CommitFlag();
    {
      auto scope = CommitFlagScope(flag);
      REQUIRE(observer.commit(0) == State::EVALUATED);
    }
    auto moved = Shared(Cell(0));
    moved = std::move(observer);
    flag.clear();
    cell->set(2);
    REQUIRE(!flag.is_raised());
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved.eval() == 2);
  }

  TEST_CASE("copy_assigning_a_moved_from") {
    auto source = Shared(Queue<int>());
    auto moved = std::move(source);
    source = moved;
    moved->push(5);
    REQUIRE(source.commit(0) == State::EVALUATED);
    REQUIRE(source.eval() == 5);
  }

  TEST_CASE("move_assigning_a_moved_from") {
    auto source = Shared(Queue<int>());
    auto moved = std::move(source);
    auto other = Shared(Queue<int>());
    source = std::move(other);
    source->push(7);
    REQUIRE(source.commit(0) == State::EVALUATED);
    REQUIRE(source.eval() == 7);
  }

  TEST_CASE("raising_between_a_box_and_its_source") {
    auto queue = Shared(Queue<int>());
    auto boxed = SharedBox<int>(queue);
    queue->push(1);
    REQUIRE(has_evaluation(boxed.commit(0)));
    REQUIRE(boxed.eval() == 1);
    REQUIRE(has_evaluation(queue.commit(0)));
    boxed.commit(1);
    queue->push(2);
    REQUIRE(has_evaluation(queue.commit(1)));
    REQUIRE(queue.eval() == 2);
    REQUIRE(has_evaluation(boxed.commit(2)));
    REQUIRE(boxed.eval() == 2);
  }
}

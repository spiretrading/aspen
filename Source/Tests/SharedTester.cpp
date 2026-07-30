#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

TEST_SUITE("Shared") {
  TEST_CASE("shared_chain") {
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

  TEST_CASE("shared_from_unique") {
    auto c = Unique(std::make_unique<Constant<int>>(5));
    auto s = Shared(std::move(c));
    REQUIRE(s.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(s.eval() == 5);
  }

  TEST_CASE("shared_constant_to_shared_box") {
    auto c = Shared(Constant(123));
    auto b = Shared<Box<int>>(c);
    REQUIRE(b.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(b.eval() == 123);
  }

  TEST_CASE("shared_move_construction") {
    auto queue = Shared(Queue<int>());
    queue->push(1);
    auto moved = Shared(std::move(queue));
    REQUIRE(moved.commit(0) == State::EVALUATED);
    REQUIRE(moved.eval() == 1);
    moved->push(2);
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved.eval() == 2);
  }

  TEST_CASE("shared_copy_assignment") {
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

  TEST_CASE("shared_move_assignment") {
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

  TEST_CASE("shared_self_assignment") {
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

  TEST_CASE("shared_is_noexcept") {
    auto cell = Shared(Cell(1));
    REQUIRE(decltype(cell)::is_noexcept);
    auto queue = Shared(Queue<int>());
    REQUIRE(!decltype(queue)::is_noexcept);
    static_assert(IsReactorOf<Shared<Cell<int>>, int>);
    static_assert(std::is_same_v<collapse_shared_t<Shared<Shared<Cell<int>>>>,
      Shared<Cell<int>>>);
  }

  TEST_CASE("shared_emplaced_from_several_arguments") {
    auto reactor = Shared<Cell<int>>(std::in_place, 7);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 7);
  }

  TEST_CASE("shared_with_none") {
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

  TEST_CASE("a_boxed_shared_caches_its_own_evaluation") {
    auto reactor = Shared(ByValueReactor(5));
    auto first = shared_box(reactor);
    auto second = shared_box(reactor);
    REQUIRE(first.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(first.eval() == 5);
    REQUIRE(second.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(second.eval() == 5);
  }

  TEST_CASE("copy_assigning_to_a_moved_from_shared") {
    auto source = Shared(Queue<int>());
    auto moved = std::move(source);
    source = moved;
    moved->push(5);
    REQUIRE(source.commit(0) == State::EVALUATED);
    REQUIRE(source.eval() == 5);
  }

  TEST_CASE("move_assigning_to_a_moved_from_shared") {
    auto source = Shared(Queue<int>());
    auto moved = std::move(source);
    auto other = Shared(Queue<int>());
    source = std::move(other);
    source->push(7);
    REQUIRE(source.commit(0) == State::EVALUATED);
    REQUIRE(source.eval() == 7);
  }
}

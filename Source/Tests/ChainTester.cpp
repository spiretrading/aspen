#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Chain.hpp"
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

TEST_SUITE("Chain") {
  TEST_CASE("two_constants") {
    auto reactor = Chain(Constant(100), Constant(200));
    auto state = reactor.commit(0);
    REQUIRE(state == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 100);
    state = reactor.commit(1);
    REQUIRE(state == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 200);
  }

  TEST_CASE("constant_initial") {
    auto reactor = Chain(Constant(911), None<int>());
    auto state = reactor.commit(0);
    REQUIRE(state == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 911);
    state = reactor.commit(1);
    REQUIRE(state == State::COMPLETE);
  }

  TEST_CASE("empty_initial") {
    auto reactor = Chain(None<int>(), Constant(911));
    auto state = reactor.commit(0);
    REQUIRE(state == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 911);
  }

  TEST_CASE("no_values") {
    auto reactor = Chain(None<int>(), None<int>());
    auto state = reactor.commit(0);
    REQUIRE(state == State::COMPLETE);
  }

  TEST_CASE("empty_continuation") {
    auto queue = Shared(Queue<int>());
    queue->push(5);
    auto reactor = Chain(queue, None<int>());
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    queue->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("immediate_completion") {
    auto queue = Shared(Queue<int>());
    auto reactor = Chain(None<int>(), queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(21);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 21);
  }

  TEST_CASE("immediate_continuation") {
    auto queue = Shared(Queue<int>());
    queue->push(21);
    auto reactor = Chain(None<int>(), queue);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 21);
  }

  TEST_CASE("initial_completing_with_a_value") {
    auto queue = Shared(Queue<int>());
    auto reactor = Chain(queue, Constant(9));
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(1);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    queue->push(2);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    queue->set_complete(3);
    REQUIRE(reactor.commit(3) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(reactor.commit(4) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 9);
  }

  TEST_CASE("series") {
    auto reactor = chain(Constant(1), Constant(2), Constant(3));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
  }

  TEST_CASE("initial_completing_without_a_value") {
    auto queue = Shared(Queue<int>());
    queue->push(5);
    auto reactor = Chain(queue, Constant(123));
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    queue->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("by_reference_children") {
    auto reactor = Chain(
      Constant(CountedValue(1)), Constant(CountedValue(2)));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    CountedValue::reset_counts();
    [[maybe_unused]] const auto& value = reactor.eval();
    REQUIRE(CountedValue::get_copies() == 0);
    REQUIRE(CountedValue::get_destructions() == 0);
  }

  TEST_CASE("by_value_children") {
    auto reactor = Chain(
      ByValueReactor(CountedValue(1)), ByValueReactor(CountedValue(2)));
    test_by_value_evaluation(reactor, 1);
  }

  TEST_CASE("by_value_continuation") {
    auto reactor = Chain(
      Constant(CountedValue(1)), ByValueReactor(CountedValue(2)));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    CountedValue::reset_counts();
    [[maybe_unused]] const auto& value = reactor.eval();
    REQUIRE(CountedValue::get_destructions() == 0);
  }

  TEST_CASE("releasing_a_completed_initial") {
    auto initial = TrackedReactor<int>(1, State::COMPLETE_EVALUATED);
    auto token = initial.get_token();
    auto reactor = Chain(std::move(initial), Constant(2));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(!token.expired());
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(token.expired());
  }

  TEST_CASE("unraised_child") {
    auto flag = CommitFlag();
    auto child = CountingReactor<int>(1, State::EVALUATED);
    auto reactor = Chain(child, Constant(2));
    auto scope = CommitFlagScope(flag);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(child.get_commits() == 1);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(child.get_commits() == 1);
  }
}

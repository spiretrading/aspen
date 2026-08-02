#include <stdexcept>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Group.hpp"
#include "Aspen/Last.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Switch.hpp"
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

TEST_SUITE("Group") {
  TEST_CASE("completion") {
    auto reactor = group(constant(123), none<int>());
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("two_constants") {
    auto reactor = group(constant(1), constant(2));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("three_children") {
    auto reactor = group(constant(1), constant(2), constant(3));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
  }

  TEST_CASE("alternating_between_children") {
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    auto reactor = group(first, second);
    REQUIRE(reactor.commit(0) == State::NONE);
    first->push(1);
    second->push(2);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(3) == State::NONE);
    first->set_complete();
    second->set_complete();
    REQUIRE(reactor.commit(4) == State::COMPLETE);
  }

  TEST_CASE("child_continuing_without_a_value") {
    auto second = Shared(Queue<int>());
    auto reactor = group(last(chain(3, 1)), second);
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("child_with_a_continuation") {
    auto reactor = group(chain(1, 2), constant(3));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("exception") {
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    auto reactor = group(first, second);
    first->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("noexcept_children") {
    auto safe = group(Shared(Cell(1)), Shared(Cell(2)));
    REQUIRE(decltype(safe)::is_noexcept);
    auto mixed = group(Shared(Cell(1)), Shared(Queue<int>()));
    REQUIRE(!decltype(mixed)::is_noexcept);
  }

  TEST_CASE("by_value_children") {
    auto reactor = Group(
      ByValueReactor(CountedValue(1)), ByValueReactor(CountedValue(2)));
    test_by_value_evaluation(reactor, 1);
  }

  TEST_CASE("by_reference_children") {
    auto reactor = Group(
      Constant(CountedValue(1)), Constant(CountedValue(2)));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    CountedValue::reset_counts();
    [[maybe_unused]] const auto& value = reactor.eval();
    REQUIRE(CountedValue::get_copies() == 0);
    REQUIRE(CountedValue::get_moves() == 0);
  }

  TEST_CASE("child_completing_with_a_value") {
    auto toggle = Shared(Queue<bool>());
    toggle->push(true);
    toggle->push(true);
    auto reactor = group(switch_(toggle, constant(5)), constant(0));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
  }

  TEST_CASE("releasing_a_child_without_a_value") {
    auto first = TrackedReactor<int>(1, State::COMPLETE);
    auto token = first.get_token();
    auto second = Shared(Queue<int>());
    auto reactor = group(std::move(first), second);
    REQUIRE(reactor.commit(0) == State::NONE);
    REQUIRE(token.expired());
  }

  TEST_CASE("releasing_an_evaluated_child_on_completion") {
    auto first = TrackedReactor<int>(123, State::COMPLETE_EVALUATED);
    auto token = first.get_token();
    auto reactor = group(std::move(first), none<int>());
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
    REQUIRE(token.expired());
  }

  TEST_CASE("releasing_a_completed_child") {
    auto first = TrackedReactor<int>(1, State::COMPLETE_EVALUATED);
    auto token = first.get_token();
    auto second = Shared(Queue<int>());
    auto reactor = group(std::move(first), second);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(!token.expired());
    second->push(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(token.expired());
  }
}

#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Group.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

TEST_SUITE("Group") {
  TEST_CASE("group_loop_complete") {
    auto reactor = group(constant(123), none<int>());
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("group_two_constants") {
    auto reactor = group(constant(1), constant(2));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("group_three_reactors") {
    auto reactor = group(constant(1), constant(2), constant(3));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
  }

  TEST_CASE("group_alternates_between_children") {
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
    REQUIRE(reactor.eval() == 2);
    first->set_complete();
    second->set_complete();
    REQUIRE(reactor.commit(4) == State::COMPLETE);
  }

  TEST_CASE("group_a_child_with_a_continuation") {
    auto reactor = group(chain(1, 2), constant(3));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("group_propagates_an_exception") {
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    auto reactor = group(first, second);
    first->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("group_is_noexcept_when_both_children_are") {
    auto safe = group(Shared(Cell(1)), Shared(Cell(2)));
    REQUIRE(decltype(safe)::is_noexcept);
    auto mixed = group(Shared(Cell(1)), Shared(Queue<int>()));
    REQUIRE(!decltype(mixed)::is_noexcept);
  }

  TEST_CASE("group_children_evaluating_by_value") {
    auto reactor = Group(
      ByValueReactor(CountedValue(1)), ByValueReactor(CountedValue(2)));
    test_evaluation_lifetime(reactor);
  }

  TEST_CASE("group_children_evaluating_by_reference_does_not_copy") {
    auto reactor = Group(
      Constant(CountedValue(1)), Constant(CountedValue(2)));
    REQUIRE(has_evaluation(reactor.commit(0)));
    CountedValue::reset_counts();
    [[maybe_unused]] const auto& value = reactor.eval();
    REQUIRE(CountedValue::get_copies() == 0);
  }

}

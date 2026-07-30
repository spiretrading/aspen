#include <utility>
#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/Concat.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Last.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

TEST_SUITE("Concat") {
  TEST_CASE("constant_then_empty") {
    auto series = Shared<Queue<SharedBox<int>>>();
    auto reactor = concat(series);
    series->push(shared_box(5));
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(reactor.eval() == 5);
    auto producer = Shared<Queue<int>>();
    series->push(shared_box(producer));
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(reactor.eval() == 5);
  }

  TEST_CASE("constant_empty_constant") {
    auto series = Shared<Queue<SharedBox<int>>>();
    series->push(shared_box(5));
    series->push(shared_box(None<int>()));
    series->push(shared_box(10));
    series->set_complete();
    auto reactor = concat(series);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(reactor.commit(1) == State::CONTINUE);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
  }

  TEST_CASE("an_empty_producer_completes") {
    auto series = Shared<Queue<SharedBox<int>>>();
    auto reactor = concat(series);
    REQUIRE(reactor.commit(0) == State::NONE);
    series->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("a_producer_that_throws_is_ignored") {
    auto series = Shared<Queue<SharedBox<int>>>();
    series->set_complete(std::runtime_error("fail"));
    auto reactor = concat(series);
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("a_child_producing_several_values") {
    auto series = Shared<Queue<SharedBox<int>>>();
    auto child = Shared(Queue<int>());
    series->set_complete(shared_box(child));
    auto reactor = concat(series);
    child->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    child->push(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(reactor.eval() == 2);
    child->set_complete();
    REQUIRE(reactor.commit(3) == State::COMPLETE);
  }

  TEST_CASE("a_child_arriving_while_another_produces") {
    auto series = Shared<Queue<SharedBox<int>>>();
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    series->push(shared_box(first));
    auto reactor = concat(series);
    first->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    series->set_complete(shared_box(second));
    second->push(10);
    first->push(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    first->set_complete();
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
    second->set_complete();
    REQUIRE(reactor.commit(3) == State::COMPLETE);
  }

  TEST_CASE("no_evaluation_continue") {
    auto series = Shared<Queue<SharedBox<int>>>();
    series->push(shared_box(10));
    series->push(shared_box(last(chain(3, 1))));
    series->set_complete();
    auto reactor = concat(series);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
    REQUIRE(reactor.commit(1) == State::CONTINUE);
    REQUIRE(reactor.eval() == 10);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("concat_children_evaluating_by_value") {
    auto reactor = Concat(Constant(ByValueReactor(CountedValue(1))));
    test_evaluation_lifetime(reactor);
  }
  TEST_CASE("a_completed_producer_is_released") {
    auto producer = TrackedReactor<SharedBox<int>>(
      shared_box(Constant(5)), State::COMPLETE_EVALUATED);
    auto token = producer.get_token();
    auto reactor = Concat(std::move(producer));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(token.expired());
  }

}

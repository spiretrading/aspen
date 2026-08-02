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
  TEST_CASE("constant_then_none") {
    auto series = Shared<Queue<SharedBox<int>>>();
    auto reactor = concat(series);
    series->push(shared_box(5));
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(reactor.commit(1) == State::NONE);
    auto producer = Shared<Queue<int>>();
    series->push(shared_box(producer));
    REQUIRE(reactor.commit(2) == State::NONE);
  }

  TEST_CASE("none_between_constants") {
    auto series = Shared<Queue<SharedBox<int>>>();
    series->push(shared_box(5));
    series->push(shared_box(None<int>()));
    series->push(shared_box(10));
    series->set_complete();
    auto reactor = concat(series);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(reactor.commit(1) == State::CONTINUE);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
  }

  TEST_CASE("empty_producer") {
    auto series = Shared<Queue<SharedBox<int>>>();
    auto reactor = concat(series);
    REQUIRE(reactor.commit(0) == State::NONE);
    series->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("producer_exception") {
    auto series = Shared<Queue<SharedBox<int>>>();
    series->set_complete(std::runtime_error("fail"));
    auto reactor = concat(series);
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("producer_exception_after_a_child") {
    auto series = Shared<Queue<SharedBox<int>>>();
    auto child = Shared(Queue<int>());
    series->push(shared_box(child));
    series->set_complete(std::runtime_error("fail"));
    auto reactor = concat(series);
    child->push(1);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::NONE);
    child->push(2);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    child->set_complete();
    REQUIRE(reactor.commit(3) == State::COMPLETE);
  }

  TEST_CASE("producer_exception_after_a_final_value") {
    auto series = Shared<Queue<SharedBox<int>>>();
    auto child = Shared(Queue<int>());
    series->push(shared_box(child));
    series->set_complete(std::runtime_error("fail"));
    auto reactor = concat(series);
    child->push(1);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::NONE);
    child->set_complete(2);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("child_with_several_values") {
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
    child->set_complete();
    REQUIRE(reactor.commit(3) == State::COMPLETE);
  }

  TEST_CASE("child_arriving_mid_stream") {
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

  TEST_CASE("continuation_without_a_value") {
    auto series = Shared<Queue<SharedBox<int>>>();
    series->push(shared_box(10));
    series->push(shared_box(last(chain(3, 1))));
    series->set_complete();
    auto reactor = concat(series);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
    REQUIRE(reactor.commit(1) == State::CONTINUE);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("by_value_child") {
    auto reactor = Concat(Constant(ByValueReactor(CountedValue(1))));
    test_by_value_evaluation(reactor, 1);
  }

  TEST_CASE("releasing_a_completed_child") {
    auto first = TrackedReactor<int>(1, State::COMPLETE_EVALUATED);
    auto token = first.get_token();
    auto second = Shared(Queue<int>());
    second->push(2);
    auto producer = Queue<SharedBox<int>>();
    producer.push(shared_box(std::move(first)));
    producer.set_complete(shared_box(second));
    auto reactor = concat(std::move(producer));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(!token.expired());
    REQUIRE(has_evaluation(reactor.commit(1)));
    REQUIRE(reactor.eval() == 2);
    REQUIRE(token.expired());
  }

  TEST_CASE("releasing_a_child_without_a_value") {
    auto child = TrackedReactor<int>(1, State::COMPLETE);
    auto token = child.get_token();
    auto producer = Queue<SharedBox<int>>();
    producer.set_complete(shared_box(std::move(child)));
    auto reactor = concat(std::move(producer));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
    REQUIRE(token.expired());
  }

  TEST_CASE("releasing_a_completed_producer") {
    auto producer = TrackedReactor<SharedBox<int>>(
      shared_box(Constant(5)), State::COMPLETE_EVALUATED);
    auto token = producer.get_token();
    auto reactor = Concat(std::move(producer));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(token.expired());
  }
}

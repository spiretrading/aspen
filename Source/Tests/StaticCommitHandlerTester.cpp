#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/StaticCommitHandler.hpp"

using namespace Aspen;

TEST_SUITE("StaticCommitHandler") {
  TEST_CASE("empty_static_commit") {
    auto reactor = StaticCommitHandler<>();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("static_immediate_complete") {
    auto reactor = StaticCommitHandler(Queue<int>());
    reactor.get<0>().push(1);
    reactor.get<0>().commit(0);
    reactor.get<0>().set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("static_complete") {
    auto queue = Shared(Queue<int>());
    auto reactor = StaticCommitHandler(queue, queue);
    reactor.get<0>()->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    reactor.get<1>()->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("static_empty_and_evaluated") {
    auto reactor = StaticCommitHandler(Queue<int>(), Queue<int>());
    REQUIRE(reactor.commit(0) == State::NONE);
    reactor.get<0>().push(123);
    REQUIRE(reactor.commit(1) == State::NONE);
    reactor.get<1>().push(321);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
  }

  TEST_CASE("static_delayed_evaluation") {
    auto reactor = StaticCommitHandler(Queue<int>(), constant(5));
    REQUIRE(reactor.commit(0) == State::NONE);
    reactor.get<0>().push(123);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.commit(2) == State::NONE);
  }

  TEST_CASE("a_child_that_continues") {
    auto reactor = StaticCommitHandler(Queue<int>(), constant(5));
    reactor.get<0>().push(1);
    reactor.get<0>().push(2);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.get<0>().eval() == 1);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.get<0>().eval() == 2);
    REQUIRE(reactor.commit(2) == State::NONE);
  }

  TEST_CASE("a_completion_suppresses_a_continuation") {
    auto queue = Shared(Queue<int>());
    auto reactor = StaticCommitHandler(queue, constant(5));
    queue->push(1);
    queue->push(2);
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.get<0>()->eval() == 2);
  }

  TEST_CASE("copying_a_handler") {
    auto handler = StaticCommitHandler(constant(1), constant(2));
    auto copy = StaticCommitHandler<Constant<int>, Constant<int>>(handler);
    REQUIRE(copy.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(copy.get<0>().eval() == 1);
    REQUIRE(copy.get<1>().eval() == 2);
  }

  TEST_CASE("moving_a_handler") {
    auto handler = StaticCommitHandler(constant(1), constant(2));
    auto moved = StaticCommitHandler(std::move(handler));
    REQUIRE(moved.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(moved.get<0>().eval() == 1);
    REQUIRE(moved.get<1>().eval() == 2);
  }

  TEST_CASE("assigning_a_handler") {
    auto handler = StaticCommitHandler(constant(1), constant(2));
    auto other = StaticCommitHandler(constant(10), constant(20));
    handler = other;
    REQUIRE(handler.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(handler.get<0>().eval() == 10);
    handler = std::move(other);
    REQUIRE(handler.get<1>().eval() == 20);
  }

  TEST_CASE("applying_over_every_reactor") {
    auto handler = StaticCommitHandler(constant(1), constant(2));
    REQUIRE(handler.commit(0) == State::COMPLETE_EVALUATED);
    auto total = apply([] (const auto&... children) {
      return (children.eval() + ...);
    }, handler);
    REQUIRE(total == 3);
    const auto& reference = handler;
    auto same = apply([] (const auto&... children) {
      return (children.eval() + ...);
    }, reference);
    REQUIRE(same == 3);
  }
}

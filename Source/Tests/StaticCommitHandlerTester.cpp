#include <utility>
#include <vector>
#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/StaticCommitHandler.hpp"

using namespace Aspen;

TEST_SUITE("StaticCommitHandler") {
  TEST_CASE("no_children") {
    auto reactor = StaticCommitHandler<>();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("immediate_completion") {
    auto reactor = StaticCommitHandler(Queue<int>());
    reactor.get<0>().push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    reactor.get<0>().set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("completion") {
    auto queue = Shared(Queue<int>());
    auto reactor = StaticCommitHandler(queue, queue);
    reactor.get<0>()->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    reactor.get<1>()->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("initial_evaluation") {
    auto reactor = StaticCommitHandler(Queue<int>(), Queue<int>());
    REQUIRE(reactor.commit(0) == State::NONE);
    reactor.get<0>().push(123);
    REQUIRE(reactor.commit(1) == State::NONE);
    reactor.get<1>().push(321);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
  }

  TEST_CASE("delayed_evaluation") {
    auto reactor = StaticCommitHandler(Queue<int>(), constant(5));
    REQUIRE(reactor.commit(0) == State::NONE);
    reactor.get<0>().push(123);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.commit(2) == State::NONE);
  }

  TEST_CASE("continuing_child") {
    auto reactor = StaticCommitHandler(Queue<int>(), constant(5));
    reactor.get<0>().push(1);
    reactor.get<0>().push(2);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.get<0>().eval() == 1);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.get<0>().eval() == 2);
    REQUIRE(reactor.commit(2) == State::NONE);
  }

  TEST_CASE("completion_with_a_continuation") {
    auto queue = Shared(Queue<int>());
    auto reactor = StaticCommitHandler(queue, constant(5));
    queue->push(1);
    queue->push(2);
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.get<0>()->eval() == 2);
  }

  TEST_CASE("copy_construction") {
    auto handler = StaticCommitHandler(constant(1), constant(2));
    auto copy = StaticCommitHandler<Constant<int>, Constant<int>>(handler);
    REQUIRE(copy.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(copy.get<0>().eval() == 1);
    REQUIRE(copy.get<1>().eval() == 2);
  }

  TEST_CASE("move_construction") {
    auto handler = StaticCommitHandler(constant(1), constant(2));
    auto moved = StaticCommitHandler(std::move(handler));
    REQUIRE(moved.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(moved.get<0>().eval() == 1);
    REQUIRE(moved.get<1>().eval() == 2);
  }

  TEST_CASE("assignment") {
    auto handler = StaticCommitHandler(constant(1), constant(2));
    auto other = StaticCommitHandler(constant(10), constant(20));
    handler = other;
    REQUIRE(handler.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(handler.get<0>().eval() == 10);
    handler = std::move(other);
    REQUIRE(handler.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(handler.get<0>().eval() == 10);
    REQUIRE(handler.get<1>().eval() == 20);
  }

  TEST_CASE("self_assignment") {
    auto handler =
      StaticCommitHandler(Constant(std::vector{1, 2, 3}), constant(4));
    auto& alias = handler;
    handler = alias;
    REQUIRE(handler.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(handler.get<0>().eval() == std::vector{1, 2, 3});
    REQUIRE(handler.get<1>().eval() == 4);
  }

  TEST_CASE("self_move_assignment") {
    auto handler =
      StaticCommitHandler(Constant(std::vector{1, 2, 3}), constant(4));
    auto& alias = handler;
    handler = std::move(alias);
    REQUIRE(handler.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(handler.get<0>().eval() == std::vector{1, 2, 3});
    REQUIRE(handler.get<1>().eval() == 4);
  }

  TEST_CASE("apply_function") {
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

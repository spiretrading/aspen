#include <doctest/doctest.h>
#include "Aspen/CommitHandler.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("CommitHandler") {
  TEST_CASE("commit_empty_commit") {
    auto reactor = CommitHandler<Box<void>>({});
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("commit_immediate_complete") {
    auto queue = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue});
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("commit_complete") {
    auto queue = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue, queue});
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    queue->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("commit_empty_and_evaluated") {
    auto queue_a = Shared(Queue<int>());
    auto queue_b = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue_a, queue_b});
    REQUIRE(reactor.commit(0) == State::NONE);
    queue_a->push(123);
    REQUIRE(reactor.commit(1) == State::NONE);
    queue_b->push(321);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
  }

  TEST_CASE("commit_delayed_evaluation") {
    auto queue = Shared(Queue<int>());
    auto children = std::vector<Box<int>>();
    children.push_back(box(queue));
    children.push_back(box(constant(123)));
    auto reactor = CommitHandler(std::move(children));
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(123);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.commit(2) == State::NONE);
  }
}

#include <catch2/catch.hpp>
#include "Aspen/CommitHandler.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_CASE("test_empty_commit", "[CommitHandler]") {
  auto reactor = CommitHandler<Box<void>>({});
  REQUIRE(reactor.commit(0) == State::COMPLETE);
}

TEST_CASE("test_immediate_complete", "[CommitHandler]") {
  auto queue = Shared(Queue<int>());
  auto reactor = CommitHandler(std::vector{queue});
  queue->set_complete();
  REQUIRE(reactor.commit(0) == State::COMPLETE);
}

TEST_CASE("test_complete", "[CommitHandler]") {
  auto queue = Shared(Queue<int>());
  auto reactor = CommitHandler(std::vector{queue, queue});
  queue->push(1);
  REQUIRE(reactor.commit(0) == State::EVALUATED);
  queue->set_complete();
  REQUIRE(reactor.commit(1) == State::COMPLETE);
}

TEST_CASE("test_empty_and_evaluated", "[CommitHandler]") {
  auto queue_a = Shared(Queue<int>());
  auto queue_b = Shared(Queue<int>());
  auto reactor = CommitHandler(std::vector{queue_a, queue_b});
  REQUIRE(reactor.commit(0) == State::NONE);
  queue_a->push(123);
  REQUIRE(reactor.commit(1) == State::NONE);
  queue_b->push(321);
  REQUIRE(reactor.commit(2) == State::EVALUATED);
}

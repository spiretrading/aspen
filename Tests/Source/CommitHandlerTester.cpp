#include <catch2/catch.hpp>
#include "Aspen/CommitHandler.hpp"
#include "Aspen/Queue.hpp"

using namespace Aspen;

TEST_CASE("test_empty_commit", "[CommitHandler]") {
  auto reactor = CommitHandler({});
  REQUIRE(reactor.commit(0) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_immediate_complete", "[CommitHandler]") {
  auto queue = Queue<int>();
  auto reactor = CommitHandler({Box<void>(&queue)});
  queue.push(1);
  queue.commit(0);
  queue.set_complete();
  REQUIRE(reactor.commit(1) == State::COMPLETE);
}

TEST_CASE("test_complete", "[CommitHandler]") {
  auto queue = Queue<int>();
  auto reactor = CommitHandler({Box<void>(&queue), Box<void>(&queue)});
  queue.push(1);
  REQUIRE(reactor.commit(0) == State::EVALUATED);
  queue.set_complete();
  REQUIRE(reactor.commit(1) == State::COMPLETE);
}

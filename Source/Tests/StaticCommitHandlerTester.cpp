#include <catch2/catch.hpp>
#include "Aspen/StaticCommitHandler.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_CASE("test_empty_static_commit", "[StaticCommitHandler]") {
  auto reactor = StaticCommitHandler<>();
  REQUIRE(reactor.commit(0) == State::COMPLETE);
}

TEST_CASE("test_static_immediate_complete", "[StaticCommitHandler]") {
  auto reactor = StaticCommitHandler(Queue<int>());
  reactor.get<0>().push(1);
  reactor.get<0>().commit(0);
  reactor.get<0>().set_complete();
  REQUIRE(reactor.commit(1) == State::COMPLETE);
}

TEST_CASE("test_static_complete", "[StaticCommitHandler]") {
  auto queue = Shared(Queue<int>());
  auto reactor = StaticCommitHandler(queue, queue);
  reactor.get<0>()->push(1);
  REQUIRE(reactor.commit(0) == State::EVALUATED);
  reactor.get<1>()->set_complete();
  REQUIRE(reactor.commit(1) == State::COMPLETE);
}

TEST_CASE("test_static_empty_and_evaluated", "[CommitHandler]") {
  auto reactor = StaticCommitHandler(Queue<int>(), Queue<int>());
  REQUIRE(reactor.commit(0) == State::NONE);
  reactor.get<0>().push(123);
  REQUIRE(reactor.commit(1) == State::NONE);
  reactor.get<1>().push(321);
  REQUIRE(reactor.commit(2) == State::EVALUATED);
}

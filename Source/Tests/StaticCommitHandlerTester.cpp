#include <doctest/doctest.h>

import Aspen;

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
}

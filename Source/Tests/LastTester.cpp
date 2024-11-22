#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Last") {
  TEST_CASE("last_constant") {
    auto reactor = last(Constant(123));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("last_none") {
    auto reactor = last(None<int>());
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("last_multiple") {
    auto queue = Shared(Queue<int>());
    auto reactor = last(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(reactor.commit(1) == State::NONE);
    queue->push(20);
    REQUIRE(reactor.commit(2) == State::NONE);
    queue->set_complete(30);
    REQUIRE(reactor.commit(3) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 30);
  }

  TEST_CASE("last_delayed_complete") {
    auto queue = Shared(Queue<int>());
    auto reactor = last(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(reactor.commit(1) == State::NONE);
    queue->push(20);
    REQUIRE(reactor.commit(2) == State::NONE);
    queue->push(30);
    REQUIRE(reactor.commit(3) == State::NONE);
    queue->set_complete();
    REQUIRE(reactor.commit(4) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 30);
  }
}

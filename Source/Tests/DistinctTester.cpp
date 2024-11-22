#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Distinct") {
  TEST_CASE("multiple") {
    auto queue = Shared(Queue<int>());
    auto reactor = distinct(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
    queue->push(20);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 20);
    queue->push(10);
    REQUIRE(reactor.commit(3) == State::NONE);
    REQUIRE(reactor.eval() == 20);
    queue->push(20);
    REQUIRE(reactor.commit(4) == State::NONE);
    REQUIRE(reactor.eval() == 20);
    queue->push(30);
    REQUIRE(reactor.commit(5) == State::EVALUATED);
    REQUIRE(reactor.eval() == 30);
  }
}

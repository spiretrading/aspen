#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Concur") {
  TEST_CASE("empty_concur") {
    auto queue = Shared(Queue<SharedBox<int>>());
    auto reactor = concur(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("loop_complete") {
    auto queue = Shared(Queue<SharedBox<int>>());
    auto reactor = concur(queue);
    queue->push(shared_box(constant(123)));
    queue->push(shared_box(none<int>()));
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.commit(2) == State::COMPLETE);
  }
}

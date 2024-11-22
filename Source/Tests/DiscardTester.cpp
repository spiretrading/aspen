#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Discard") {
  TEST_CASE("flipping_discard") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    series->push(1);
    series->push(2);
    series->push(3);
    series->push(4);
    auto reactor = discard(toggle, series);
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    toggle->push(false);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    toggle->push(true);
    REQUIRE(reactor.commit(2) == State::CONTINUE);
    REQUIRE(reactor.eval() == 2);
    toggle->push(false);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 4);
  }
}

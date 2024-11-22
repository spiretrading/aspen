#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Perpetual") {
  TEST_CASE("perpetual") {
    auto reactor = Perpetual();
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(100) == State::CONTINUE_EVALUATED);
  }
}

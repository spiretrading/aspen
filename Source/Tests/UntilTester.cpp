#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Until") {
  TEST_CASE("until_none") {
    auto reactor = Until(Constant(true), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }
}

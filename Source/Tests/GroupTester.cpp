#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Group") {
  TEST_CASE("group_loop_complete") {
    auto reactor = group(constant(123), none<int>());
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }
}

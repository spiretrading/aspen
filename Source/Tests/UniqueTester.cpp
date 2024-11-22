#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Unique") {
  TEST_CASE("unique_constant") {
    auto constant = Unique(new Constant(123));
    REQUIRE(constant.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(constant.eval() == 123);
  }
}

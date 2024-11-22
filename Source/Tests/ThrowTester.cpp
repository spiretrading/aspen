#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Throw") {
  TEST_CASE("throw") {
    auto constant = Throw<int>(std::runtime_error(""));
    REQUIRE(constant.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(constant.eval(), std::runtime_error);
  }
}

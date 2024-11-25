#include <doctest/doctest.h>

import std;
import Aspen;

using namespace Aspen;

TEST_SUITE("None") {
  TEST_CASE("none_int") {
    auto none = None<int>();
    REQUIRE(none.commit(0) == State::COMPLETE);
    REQUIRE_THROWS_AS(none.eval(), std::runtime_error);
  }
}

#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"
#include "Aspen/Until.hpp"

using namespace Aspen;

TEST_SUITE("Until") {
  TEST_CASE("until_none") {
    auto reactor = Until(Constant(true), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }
}

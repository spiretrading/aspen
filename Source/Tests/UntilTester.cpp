#include <catch2/catch.hpp>
#include "Aspen/Constant.hpp"
#include "Aspen/Until.hpp"

using namespace Aspen;

TEST_CASE("test_until_none", "[Switch]") {
  auto reactor = Until(Constant(true), Constant(10));
  REQUIRE(reactor.commit(0) == State::COMPLETE);
}

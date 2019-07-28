#include <catch2/catch.hpp>
#include "Aspen/Constant.hpp"
#include "Aspen/Switch.hpp"

using namespace Aspen;

TEST_CASE("test_empty_switch", "[Switch]") {
  auto reactor = Switch(Constant(false), Constant(10));
  REQUIRE(reactor.commit(0) == State::COMPLETE_EMPTY);
}

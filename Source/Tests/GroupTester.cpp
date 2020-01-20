#include <catch2/catch.hpp>
#include "Aspen/Aspen.hpp"

using namespace Aspen;

TEST_CASE("test_group_loop_complete", "[Group]") {
  auto reactor = group(constant(123), none<int>());
  REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
  REQUIRE(reactor.eval() == 123);
  REQUIRE(reactor.commit(1) == State::COMPLETE);
}

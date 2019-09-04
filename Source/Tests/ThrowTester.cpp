#include <exception>
#include <catch2/catch.hpp>
#include "Aspen/Throw.hpp"

using namespace Aspen;

TEST_CASE("test_throw", "[Throw]") {
  auto constant = Throw<int>(std::runtime_error(""));
  REQUIRE(constant.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(constant.eval(), std::runtime_error);
}

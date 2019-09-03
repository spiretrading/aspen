#include <catch2/catch.hpp>
#include "Aspen/Constant.hpp"
#include "Aspen/Unique.hpp"

using namespace Aspen;

TEST_CASE("test_unique_constant", "[Unique]") {
  auto constant = Unique(new Constant(123));
  REQUIRE(constant.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(constant.eval() == 123);
}

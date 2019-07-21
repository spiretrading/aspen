#include <catch2/catch.hpp>
#include "Aspen/None.hpp"

using namespace Aspen;

TEST_CASE("test_none_int", "[None]") {
  auto none = None<int>();
  REQUIRE(none.commit(0) == State::COMPLETE_EMPTY);
  REQUIRE_THROWS_AS(none.eval(), std::runtime_error);
}

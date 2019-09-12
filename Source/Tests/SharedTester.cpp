#include <catch2/catch.hpp>
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_CASE("test_shared_chain", "[Shared]") {
  auto s1 = Shared(Chain(constant(10), constant(9)));
  auto s2 = s1;
  REQUIRE(s1.commit(0) == State::CONTINUE_EVALUATED);
  REQUIRE(s1.eval() == 10);
  REQUIRE(s2.commit(0) == State::CONTINUE_EVALUATED);
  REQUIRE(s2.eval() == 10);
  REQUIRE(s1.commit(1) == State::COMPLETE_EVALUATED);
  REQUIRE(s1.eval() == 9);
  REQUIRE(s2.commit(1) == State::COMPLETE_EVALUATED);
  REQUIRE(s2.eval() == 9);
}
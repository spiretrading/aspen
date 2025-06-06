#include <doctest/doctest.h>
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Count.hpp"

using namespace Aspen;

TEST_SUITE("Count") {
  TEST_CASE("count_chain") {
    auto counter = count(chain(5, 6));
    REQUIRE(counter.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(counter.eval() == 1);
    REQUIRE(counter.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(counter.eval() == 2);
  }
}

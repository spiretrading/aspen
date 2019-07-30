#include <catch2/catch.hpp>
#include "Aspen/Discard.hpp"
#include "Aspen/Queue.hpp"

using namespace Aspen;

TEST_CASE("test_flipping_discard", "[Discard]") {
  auto toggle = Queue<bool>();
  auto series = Queue<int>();
  series.push(1);
  series.push(2);
  series.push(3);
  series.push(4);
  auto reactor = discard(&toggle, &series);
  REQUIRE(reactor.commit(0) == State::CONTINUE_EMPTY);
  toggle.push(true);
  REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
  REQUIRE(reactor.eval() == 2);
  toggle.push(false);
  REQUIRE(reactor.commit(2) == State::CONTINUE);
  REQUIRE(reactor.eval() == 2);
  toggle.push(true);
  REQUIRE(reactor.commit(3) == State::EVALUATED);
  REQUIRE(reactor.eval() == 4);
}

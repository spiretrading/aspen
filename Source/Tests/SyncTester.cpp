#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Sync") {
  TEST_CASE("empty_sync") {
    auto value = 0;
    auto reactor = Sync(value, none<int>());
    REQUIRE(reactor.commit(0) == State::COMPLETE);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(value == 0);
  }

  TEST_CASE("single_sync") {
    auto value = 0;
    auto reactor = Sync(value, constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(value == 5);
  }

  TEST_CASE("exception_sync") {
    auto value = 0;
    auto reactor = Sync(value, chain(throws<int>(std::runtime_error("fail")),
      constant(12)));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(value == 0);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 12);
    REQUIRE(value == 12);
  }
}

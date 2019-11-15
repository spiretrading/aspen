#include <catch2/catch.hpp>
#include "Aspen/Chain.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Sync.hpp"
#include "Aspen/Throw.hpp"

using namespace Aspen;

TEST_CASE("test_empty_sync", "[Sync]") {
  auto value = 0;
  auto reactor = Sync(value, none<int>());
  REQUIRE(reactor.commit(0) == State::COMPLETE);
  REQUIRE(reactor.eval() == 0);
  REQUIRE(value == 0);
}

TEST_CASE("test_single_sync", "[Sync]") {
  auto value = 0;
  auto reactor = Sync(value, constant(5));
  REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 5);
  REQUIRE(value == 5);
}

TEST_CASE("test_exception_sync", "[Sync]") {
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

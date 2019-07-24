#include <catch2/catch.hpp>
#include "Aspen/Range.hpp"

using namespace Aspen;

TEST_CASE("test_backward_range", "[Range]") {
  auto reactor = range(constant(10), constant(9));
  REQUIRE(reactor.commit(0) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_empty_range", "[Range]") {
  auto reactor = range(constant(1), constant(1));
  REQUIRE(reactor.commit(0) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_single_range", "[Range]") {
  auto reactor = range(constant(1), constant(2));
  REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 1);
}

TEST_CASE("test_double_range", "[Range]") {
  auto reactor = range(constant(10), constant(12));
  REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
  REQUIRE(reactor.eval() == 10);
  REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 11);
}

#include <catch2/catch.hpp>
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Switch.hpp"

using namespace Aspen;

TEST_CASE("test_empty_switch", "[Switch]") {
  auto reactor = Switch(Constant(false), Constant(10));
  REQUIRE(reactor.commit(0) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_true_switch", "[Switch]") {
  auto reactor = Switch(Constant(true), Constant(10));
  REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 10);
}

TEST_CASE("test_none_switch", "[Switch]") {
  auto reactor = Switch(none<bool>(), Constant(10));
  REQUIRE(reactor.commit(0) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_flipping_switch", "[Switch]") {
  auto toggle = Shared(Queue<bool>());
  auto series = Shared(Queue<int>());
  auto reactor = Switch(toggle, series);
  REQUIRE(reactor.commit(0) == State::EMPTY);
  toggle->push(true);
  REQUIRE(reactor.commit(1) == State::EMPTY);
  series->push(321);
  REQUIRE(reactor.commit(2) == State::EVALUATED);
  REQUIRE(reactor.eval() == 321);
  toggle->push(false);
  REQUIRE(reactor.commit(3) == State::NONE);
  REQUIRE(reactor.eval() == 321);
  toggle->push(true);
  REQUIRE(reactor.commit(4) == State::EVALUATED);
  REQUIRE(reactor.eval() == 321);
}

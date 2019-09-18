#include <catch2/catch.hpp>
#include "Aspen/Box.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/Concat.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Last.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_CASE("test_constant_then_empty", "[Concat]") {
  auto series = Shared<Queue<Box<int>>>();
  auto reactor = concat(series);
  series->push(Box(5));
  REQUIRE(reactor.commit(0) == State::EVALUATED);
  REQUIRE(reactor.eval() == 5);
  REQUIRE(reactor.commit(1) == State::NONE);
  REQUIRE(reactor.eval() == 5);
  auto producer = Shared<Queue<int>>();
  series->push(Box(producer));
  REQUIRE(reactor.commit(2) == State::NONE);
  REQUIRE(reactor.eval() == 5);
}

TEST_CASE("test_constant_empty_constant", "[Concat]") {
  auto series = Shared<Queue<Box<int>>>();
  series->push(Box(5));
  series->push(Box(None<int>()));
  series->push(Box(10));
  series->set_complete();
  auto reactor = concat(series);
  REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
  REQUIRE(reactor.eval() == 5);
  REQUIRE(reactor.commit(1) == State::CONTINUE);
  REQUIRE(reactor.eval() == 5);
  REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 10);
}

TEST_CASE("test_no_evaluation_continue", "[Concat]") {
  auto series = Shared<Queue<Box<int>>>();
  series->push(Box(10));
  series->push(Box(last(chain(3, 1))));
  series->set_complete();
  auto reactor = concat(series);
  REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
  REQUIRE(reactor.eval() == 10);
  REQUIRE(reactor.commit(1) == State::CONTINUE);
  REQUIRE(reactor.eval() == 10);
  REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 1);
}

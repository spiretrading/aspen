#include <catch2/catch.hpp>
#include "Aspen/Constant.hpp"
#include "Aspen/First.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_CASE("test_first_constant", "[First]") {
  auto reactor = first(Constant(123));
  REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 123);
}

TEST_CASE("test_first_none", "[First]") {
  auto reactor = first(None<int>());
  REQUIRE(reactor.commit(0) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_first_multiple", "[First]") {
  auto queue = Shared(Queue<int>());
  auto reactor = first(queue);
  REQUIRE(reactor.commit(0) == State::EMPTY);
  queue->push(10);
  REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 10);
  queue->push(20);
  REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 10);
}

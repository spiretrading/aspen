#include <catch2/catch.hpp>
#include "Aspen/Aspen.hpp"

using namespace Aspen;

TEST_CASE("test_empty_group", "[Group]") {
  auto queue = Shared(Queue<Box<int>>());
  auto reactor = group(queue);
  REQUIRE(reactor.commit(0) == State::NONE);
  queue->set_complete();
  REQUIRE(reactor.commit(1) == State::COMPLETE);
}

TEST_CASE("test_loop_complete", "[Group]") {
  auto queue = Shared(Queue<Box<int>>());
  auto reactor = group(queue);
  queue->push(box(constant(123)));
  queue->push(box(none<int>()));
  queue->set_complete();
  REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
  REQUIRE(reactor.eval() == 123);
  REQUIRE(reactor.commit(1) == State::NONE);
  REQUIRE(reactor.eval() == 123);
  REQUIRE(reactor.commit(2) == State::COMPLETE);
}

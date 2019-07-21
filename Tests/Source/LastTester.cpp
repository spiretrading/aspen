#include <catch2/catch.hpp>
#include "Aspen/Constant.hpp"
#include "Aspen/Last.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"

using namespace Aspen;

TEST_CASE("test_last_constant", "[Last]") {
  auto reactor = last(Constant(123));
  REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 123);
}

TEST_CASE("test_last_none", "[Last]") {
  auto reactor = last(None<int>());
  REQUIRE(reactor.commit(0) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_last_multiple", "[Last]") {
  auto trigger = Trigger();
  auto queue = Queue<int>();
  auto reactor = last(&queue);
  REQUIRE(reactor.commit(0) == State::NONE);
  queue.push(10);
  REQUIRE(reactor.commit(1) == State::NONE);
  queue.push(20);
  REQUIRE(reactor.commit(2) == State::NONE);
  queue.set_complete(30);
  REQUIRE(reactor.commit(3) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 30);
}

TEST_CASE("test_last_delayed_complete", "[Last]") {
  auto trigger = Trigger();
  auto queue = Queue<int>();
  auto reactor = last(&queue);
  REQUIRE(reactor.commit(0) == State::NONE);
  queue.push(10);
  REQUIRE(reactor.commit(1) == State::NONE);
  queue.push(20);
  REQUIRE(reactor.commit(2) == State::NONE);
  queue.push(30);
  REQUIRE(reactor.commit(3) == State::NONE);
  queue.set_complete();
  REQUIRE(reactor.commit(4) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 30);
}

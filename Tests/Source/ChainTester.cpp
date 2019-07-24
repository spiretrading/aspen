#include <catch2/catch.hpp>
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/None.hpp"

using namespace Aspen;

TEST_CASE("test_constant_chain", "[Chain]") {
  auto reactor = Chain(Constant(100), Constant(200));
  auto state = reactor.commit(0);
  REQUIRE(state == State::CONTINUE_EVALUATED);
  REQUIRE(reactor.eval() == 100);
  state = reactor.commit(1);
  REQUIRE(state == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 200);
}

TEST_CASE("test_single_chain", "[Chain]") {
  auto reactor = Chain(Constant(911), None<int>());
  auto state = reactor.commit(0);
  REQUIRE(state == State::CONTINUE_EVALUATED);
  REQUIRE(reactor.eval() == 911);
  state = reactor.commit(1);
  REQUIRE(state == State::COMPLETE);
  REQUIRE(reactor.eval() == 911);
}

TEST_CASE("test_chain_immediate_transition", "[Chain]") {
  auto reactor = Chain(None<int>(), Constant(911));
  auto state = reactor.commit(0);
  REQUIRE(state == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 911);
}

TEST_CASE("test_empty_chain", "[Chain]") {
  auto reactor = Chain(None<int>(), None<int>());
  auto state = reactor.commit(0);
  REQUIRE(state == State::COMPLETE_EMPTY);
}

TEST_CASE("test_chain_with_initial_complete", "[Chain]") {
  auto queue = Queue<int>();
  queue.push(5);
  queue.commit(0);
  queue.set_complete();
  auto reactor = Chain(&queue, Constant(123));
  REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
  REQUIRE(reactor.eval() == 5);
  REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 123);
}

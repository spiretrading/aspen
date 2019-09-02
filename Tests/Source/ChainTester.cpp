#include <catch2/catch.hpp>
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

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

TEST_CASE("test_chain_initial_complete_none", "[Chain]") {
  auto queue = Shared<Queue<int>>();
  queue->push(5);
  queue->commit(0);
  auto reactor = Chain(queue, None<int>());
  REQUIRE(reactor.commit(1) == State::EVALUATED);
  REQUIRE(reactor.eval() == 5);
  queue->set_complete();
  REQUIRE(reactor.commit(2) == State::COMPLETE);
  REQUIRE(reactor.eval() == 5);
}

TEST_CASE("test_chain_immediate_complete", "[Chain]") {
  auto queue = Shared<Queue<int>>();
  auto reactor = Chain(None<int>(), queue);
  REQUIRE(reactor.commit(0) == State::EMPTY);
  queue->push(21);
  REQUIRE(reactor.commit(1) == State::EVALUATED);
  REQUIRE(reactor.eval() == 21);
}

TEST_CASE("test_chain_immediate_continue", "[Chain]") {
  auto queue = Shared<Queue<int>>();
  queue->push(21);
  auto reactor = Chain(None<int>(), queue);
  REQUIRE(reactor.commit(0) == State::EVALUATED);
  REQUIRE(reactor.eval() == 21);
}

TEST_CASE("test_chain_initial_complete", "[Chain]") {
  auto queue = Shared<Queue<int>>();
  queue->push(5);
  queue->commit(0);
  auto reactor = Chain(queue, Constant(123));
  REQUIRE(reactor.commit(1) == State::EVALUATED);
  REQUIRE(reactor.eval() == 5);
  queue->set_complete();
  REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 123);
}

#include <doctest/doctest.h>
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Chain") {
  TEST_CASE("constant_chain") {
    auto reactor = Chain(Constant(100), Constant(200));
    auto state = reactor.commit(0);
    REQUIRE(state == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 100);
    state = reactor.commit(1);
    REQUIRE(state == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 200);
  }

  TEST_CASE("single_chain") {
    auto reactor = Chain(Constant(911), None<int>());
    auto state = reactor.commit(0);
    REQUIRE(state == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 911);
    state = reactor.commit(1);
    REQUIRE(state == State::COMPLETE);
    REQUIRE(reactor.eval() == 911);
  }

  TEST_CASE("chain_immediate_transition") {
    auto reactor = Chain(None<int>(), Constant(911));
    auto state = reactor.commit(0);
    REQUIRE(state == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 911);
  }

  TEST_CASE("empty_chain") {
    auto reactor = Chain(None<int>(), None<int>());
    auto state = reactor.commit(0);
    REQUIRE(state == State::COMPLETE);
  }

  TEST_CASE("chain_initial_complete_none") {
    auto queue = Shared(Queue<int>());
    queue->push(5);
    queue.commit(0);
    auto reactor = Chain(queue, None<int>());
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    queue->set_complete();
    REQUIRE(reactor.commit(2) == State::COMPLETE);
    REQUIRE(reactor.eval() == 5);
  }

  TEST_CASE("chain_immediate_complete") {
    auto queue = Shared(Queue<int>());
    auto reactor = Chain(None<int>(), queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(21);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 21);
  }

  TEST_CASE("chain_immediate_continue") {
    auto queue = Shared(Queue<int>());
    queue->push(21);
    auto reactor = Chain(None<int>(), queue);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 21);
  }

  TEST_CASE("chaining_an_initial_that_produces_then_completes") {
    auto queue = Shared(Queue<int>());
    auto reactor = Chain(queue, Constant(9));
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(1);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    queue->push(2);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    queue->set_complete(3);
    REQUIRE(reactor.commit(3) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(reactor.commit(4) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 9);
  }

  TEST_CASE("chaining_a_series") {
    auto reactor = chain(Constant(1), Constant(2), Constant(3));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
  }

  TEST_CASE("chain_initial_complete") {
    auto queue = Shared(Queue<int>());
    queue->push(5);
    queue.commit(0);
    auto reactor = Chain(queue, Constant(123));
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    queue->set_complete();
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }
}

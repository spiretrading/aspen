#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/StateReactor.hpp"

using namespace Aspen;

TEST_SUITE("StateReactor") {
  TEST_CASE("none_state") {
    auto reactor = StateReactor(none<int>());
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == State::COMPLETE);
  }

  TEST_CASE("immediate_completion") {
    auto reactor = StateReactor(constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == State::COMPLETE_EVALUATED);
  }

  TEST_CASE("series_then_complete") {
    auto queue = Shared(Queue<int>());
    auto reactor = StateReactor(queue);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == State::NONE);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(reactor.eval() == State::NONE);
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(reactor.eval() == State::NONE);
    queue->push(123);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == State::EVALUATED);
    queue->push(5);
    queue->push(10);
    REQUIRE(reactor.commit(4) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(5) == State::EVALUATED);
    REQUIRE(reactor.eval() == State::EVALUATED);
    queue->set_complete();
    REQUIRE(reactor.commit(6) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == State::COMPLETE);
  }
}

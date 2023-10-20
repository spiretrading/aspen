#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"
#include "Aspen/Previous.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Distinct") {
  TEST_CASE("empty") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(queue);
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("single") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(queue);
    queue->set_complete(123);
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("single_delay") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(queue);
    queue->push(321);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 321);
  }

  TEST_CASE("multiple") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(reactor.commit(1) == State::NONE);
    queue->push(20);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
    queue->push(30);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 20);
    queue->set_complete();
    REQUIRE(reactor.commit(4) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 30);
  }
}

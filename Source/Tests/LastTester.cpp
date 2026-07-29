#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Last.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Last") {
  TEST_CASE("last_constant") {
    auto reactor = last(Constant(123));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("last_value") {
    auto reactor = last(123);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("last_none") {
    auto reactor = last(None<int>());
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("last_multiple") {
    auto queue = Shared(Queue<int>());
    auto reactor = last(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(reactor.commit(1) == State::NONE);
    queue->push(20);
    REQUIRE(reactor.commit(2) == State::NONE);
    queue->set_complete(30);
    REQUIRE(reactor.commit(3) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 30);
  }

  TEST_CASE("last_delayed_complete") {
    auto queue = Shared(Queue<int>());
    auto reactor = last(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(reactor.commit(1) == State::NONE);
    queue->push(20);
    REQUIRE(reactor.commit(2) == State::NONE);
    queue->push(30);
    REQUIRE(reactor.commit(3) == State::NONE);
    queue->set_complete();
    REQUIRE(reactor.commit(4) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 30);
  }

  TEST_CASE("last_completing_without_a_value") {
    auto queue = Shared(Queue<int>());
    auto reactor = last(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("last_a_noexcept_source") {
    auto cell = Shared(Cell(1));
    auto reactor = last(cell);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::NONE);
    cell->set(2);
    REQUIRE(reactor.commit(1) == State::NONE);
    cell->set_complete(3);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
  }

  TEST_CASE("last_propagates_an_exception") {
    auto queue = Shared(Queue<int>());
    auto reactor = last(queue);
    queue->push(10);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }
}

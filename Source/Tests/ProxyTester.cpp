#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Proxy.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Proxy") {
  TEST_CASE("proxy_without_a_reactor") {
    auto reactor = proxy<Shared<Queue<int>>>();
    REQUIRE(reactor.commit(0) == State::NONE);
    REQUIRE(reactor.commit(1) == State::NONE);
  }

  TEST_CASE("proxy_forwards_to_its_reactor") {
    auto queue = Shared(Queue<int>());
    auto reactor = Proxy<Shared<Queue<int>>>();
    reactor.set_reactor(queue);
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    queue->push(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("proxy_stays_complete") {
    auto queue = Shared(Queue<int>());
    auto reactor = Proxy<Shared<Queue<int>>>();
    reactor.set_reactor(queue);
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("proxy_set_after_a_commit") {
    auto queue = Shared(Queue<int>());
    auto reactor = Proxy<Shared<Queue<int>>>();
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(7);
    reactor.set_reactor(queue);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 7);
  }

  TEST_CASE("proxy_a_reactor_that_fails") {
    auto queue = Shared(Queue<int>());
    auto reactor = Proxy<Shared<Queue<int>>>();
    reactor.set_reactor(queue);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("proxy_a_reactor_that_cannot_throw") {
    auto cell = Shared(Cell(1));
    auto reactor = Proxy<Shared<Cell<int>>>();
    REQUIRE(decltype(reactor)::is_noexcept);
    reactor.set_reactor(cell);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    cell->set(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }
}

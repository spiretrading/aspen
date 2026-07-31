#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Cell.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Proxy.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

TEST_SUITE("Proxy") {
  TEST_CASE("without_a_reactor") {
    auto reactor = proxy<Shared<Queue<int>>>();
    REQUIRE(reactor.commit(0) == State::NONE);
    REQUIRE(reactor.commit(1) == State::NONE);
  }

  TEST_CASE("forwarding") {
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

  TEST_CASE("completion") {
    auto queue = Shared(Queue<int>());
    auto reactor = Proxy<Shared<Queue<int>>>();
    reactor.set_reactor(queue);
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("set_after_a_commit") {
    auto queue = Shared(Queue<int>());
    auto reactor = Proxy<Shared<Queue<int>>>();
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(7);
    reactor.set_reactor(queue);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 7);
  }

  TEST_CASE("exception") {
    auto queue = Shared(Queue<int>());
    auto reactor = Proxy<Shared<Queue<int>>>();
    reactor.set_reactor(queue);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("noexcept_reactor") {
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

  TEST_CASE("cyclic_reactor") {
    auto reactor = Shared(Proxy<SharedBox<int>>());
    reactor->set_reactor(shared_box(lift([] (int value) noexcept {
      return value + 1;
    }, reactor)));
    REQUIRE(reactor.commit(0) == State::NONE);
    REQUIRE(reactor.commit(1) == State::NONE);
  }

  TEST_CASE("by_value_reactor") {
    auto reactor = Proxy<ByValueReactor<CountedValue>>();
    reactor.set_reactor(ByValueReactor(CountedValue(1)));
    test_evaluation_lifetime(reactor);
  }
}

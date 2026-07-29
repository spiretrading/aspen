#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Override.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Override") {
  TEST_CASE("override_an_empty_producer") {
    auto series = Shared<Queue<SharedBox<int>>>();
    auto reactor = override(series);
    REQUIRE(reactor.commit(0) == State::NONE);
    series->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("override_a_producer_that_fails") {
    auto series = Shared<Queue<SharedBox<int>>>();
    series->set_complete(std::runtime_error("fail"));
    auto reactor = override(series);
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("override_a_single_reactor") {
    auto series = Shared<Queue<SharedBox<int>>>();
    auto reactor = override(series);
    auto only = Shared<Queue<int>>();
    series->push(shared_box(only));
    series->set_complete();
    only->push(7);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 7);
    only->set_complete(8);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 8);
  }

  TEST_CASE("a_later_reactor_overrides_an_earlier_one") {
    auto series = Shared<Queue<SharedBox<int>>>();
    auto reactor = override(series);
    auto first = Shared<Queue<int>>();
    auto second = Shared<Queue<int>>();
    series->push(shared_box(first));
    first->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    first->push(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    series->push(shared_box(second));
    second->push(10);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
    REQUIRE(reactor.commit(3) == State::NONE);
    first->push(3);
    REQUIRE(reactor.commit(4) == State::NONE);
    REQUIRE(reactor.eval() == 10);
    second->push(20);
    REQUIRE(reactor.commit(5) == State::EVALUATED);
    REQUIRE(reactor.eval() == 20);
  }
}

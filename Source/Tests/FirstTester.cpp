#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/First.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("First") {
  TEST_CASE("constant") {
    auto reactor = first(Constant(123));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("value") {
    auto reactor = first(123);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("none") {
    auto reactor = first(None<int>());
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("delayed_value") {
    auto queue = Shared(Queue<int>());
    auto reactor = first(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
  }

  TEST_CASE("noexcept_source") {
    auto cell = Shared(Cell(1));
    auto reactor = first(cell);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("continuation") {
    auto reactor = first(chain(1, 2));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("exception") {
    auto queue = Shared(Queue<int>());
    auto reactor = first(queue);
    REQUIRE(!decltype(reactor)::is_noexcept);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }
}

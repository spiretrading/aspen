#include <stdexcept>
#include <string>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Unconsecutive.hpp"

using namespace Aspen;

TEST_SUITE("Unconsecutive") {
  TEST_CASE("repeated_values") {
    auto queue = Shared(Queue<int>());
    auto reactor = unconsecutive(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(1);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    queue->push(1);
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(reactor.eval() == 1);
    queue->push(2);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    queue->push(2);
    REQUIRE(reactor.commit(4) == State::NONE);
    queue->push(1);
    REQUIRE(reactor.commit(5) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("constant") {
    auto reactor = unconsecutive(Constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
  }

  TEST_CASE("noexcept_source") {
    auto cell = Shared(Cell(1));
    auto reactor = unconsecutive(cell);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    cell->set(1);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(reactor.eval() == 1);
    cell->set(2);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("allocating_value") {
    auto cell = Shared(Cell(std::string("a")));
    auto reactor = unconsecutive(cell);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == "a");
    cell->set("a");
    REQUIRE(reactor.commit(1) == State::NONE);
    cell->set("b");
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == "b");
  }

  TEST_CASE("completion_with_a_repeat") {
    auto queue = Shared(Queue<int>());
    auto reactor = unconsecutive(queue);
    queue->push(10);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
    queue->set_complete(10);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("continuation_with_a_repeat") {
    auto queue = Shared(Queue<int>());
    auto reactor = unconsecutive(queue);
    queue->push(10);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
    queue->push(10);
    queue->push(20);
    REQUIRE(reactor.commit(1) == State::CONTINUE);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 20);
  }

  TEST_CASE("exception") {
    auto queue = Shared(Queue<int>());
    auto reactor = unconsecutive(queue);
    REQUIRE(!decltype(reactor)::is_noexcept);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }
}

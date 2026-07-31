#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Distinct.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Previous.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Previous") {
  TEST_CASE("completion_without_a_value") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(queue);
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("single_value") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(queue);
    queue->set_complete(123);
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("delayed_completion") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(queue);
    queue->push(321);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 321);
  }

  TEST_CASE("series_of_values") {
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

  TEST_CASE("noexcept_source") {
    auto cell = Shared(Cell(1));
    auto reactor = previous(cell);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::NONE);
    cell->set(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    cell->set(3);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("exception_before_a_value") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(queue);
    REQUIRE(!decltype(reactor)::is_noexcept);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("exception_after_a_value") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(queue);
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("exception_between_values") {
    auto queue = Shared(Queue<int>());
    auto source = Shared(lift([] (int value) {
      if(value == 3) {
        throw std::runtime_error("fail");
      }
      return value;
    }, queue));
    auto reactor = previous(source);
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    queue->push(3);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    queue->push(4);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("suppressed_evaluation") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(distinct(queue));
    queue->push(5);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(6);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    queue->push(6);
    REQUIRE(reactor.commit(2) == State::NONE);
  }

  TEST_CASE("completion_with_a_value") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(queue);
    queue->push(1);
    queue->set_complete(2);
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("completion_with_a_queued_value") {
    auto queue = Shared(Queue<int>());
    auto reactor = previous(queue);
    queue->push(1);
    queue->push(2);
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }
}

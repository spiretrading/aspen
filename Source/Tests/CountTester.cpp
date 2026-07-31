#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Count.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Count") {
  TEST_CASE("chain") {
    auto counter = count(chain(5, 6));
    REQUIRE((std::is_same_v<decltype(counter)::Type, std::uint64_t>));
    REQUIRE(decltype(counter)::is_noexcept);
    REQUIRE(counter.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(counter.eval() == 1);
    REQUIRE(counter.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(counter.eval() == 2);
  }

  TEST_CASE("constant") {
    auto counter = count(Constant(5));
    REQUIRE(counter.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(counter.eval() == 1);
  }

  TEST_CASE("noexcept_source") {
    auto cell = Shared(Cell(1));
    auto counter = count(cell);
    REQUIRE(decltype(counter)::is_noexcept);
    REQUIRE(counter.commit(0) == State::EVALUATED);
    REQUIRE(counter.eval() == 1);
    cell->set(1);
    REQUIRE(counter.commit(1) == State::EVALUATED);
    REQUIRE(counter.eval() == 2);
  }

  TEST_CASE("advancing") {
    auto queue = Shared(Queue<int>());
    auto counter = count(queue);
    REQUIRE(!decltype(counter)::is_noexcept);
    REQUIRE(counter.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(counter.commit(1) == State::EVALUATED);
    REQUIRE(counter.eval() == 1);
    REQUIRE(counter.commit(2) == State::NONE);
    REQUIRE(counter.eval() == 1);
    queue->push(10);
    REQUIRE(counter.commit(3) == State::EVALUATED);
    REQUIRE(counter.eval() == 2);
    queue->set_complete();
    REQUIRE(counter.commit(4) == State::COMPLETE);
    REQUIRE(counter.eval() == 2);
  }

  TEST_CASE("exception_between_values") {
    auto queue = Shared(Queue<int>());
    auto counter = count(lift([] (int value) {
      if(value == 20) {
        throw std::runtime_error("fail");
      }
      return value;
    }, queue));
    queue->push(10);
    REQUIRE(counter.commit(0) == State::EVALUATED);
    REQUIRE(counter.eval() == 1);
    queue->push(20);
    REQUIRE(counter.commit(1) == State::EVALUATED);
    REQUIRE_THROWS_AS(counter.eval(), std::runtime_error);
    queue->push(30);
    REQUIRE(counter.commit(2) == State::EVALUATED);
    REQUIRE(counter.eval() == 3);
  }

  TEST_CASE("exception") {
    auto queue = Shared(Queue<int>());
    auto counter = count(queue);
    queue->push(10);
    REQUIRE(counter.commit(0) == State::EVALUATED);
    REQUIRE(counter.eval() == 1);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(counter.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(counter.eval(), std::runtime_error);
  }
}

#include <cstddef>
#include <stdexcept>
#include <string>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Distinct.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

namespace {
  struct Point {
    int m_x;
    int m_y;

    Point(int x, int y) noexcept
      : m_x(x),
        m_y(y) {}
  };
}

template<>
struct Aspen::DistinctHash<Point> {
  std::size_t operator ()(const Point& value) const noexcept {
    return static_cast<std::size_t>(value.m_x);
  }
};

template<>
struct Aspen::DistinctEquality<Point> {
  bool operator ()(const Point& left, const Point& right) const noexcept {
    return left.m_x == right.m_x;
  }
};

TEST_SUITE("Distinct") {
  TEST_CASE("repeated_values") {
    auto queue = Shared(Queue<int>());
    auto reactor = distinct(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
    queue->push(20);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 20);
    queue->push(10);
    REQUIRE(reactor.commit(3) == State::NONE);
    queue->push(20);
    REQUIRE(reactor.commit(4) == State::NONE);
    queue->push(30);
    REQUIRE(reactor.commit(5) == State::EVALUATED);
    REQUIRE(reactor.eval() == 30);
  }

  TEST_CASE("constant") {
    auto reactor = distinct(Constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
  }

  TEST_CASE("noexcept_source") {
    auto cell = Shared(Cell(1));
    auto reactor = distinct(cell);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    cell->set(1);
    REQUIRE(reactor.commit(1) == State::NONE);
    cell->set(2);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    cell->set(1);
    REQUIRE(reactor.commit(3) == State::NONE);
  }

  TEST_CASE("specialized_functors") {
    auto cell = Shared(Cell(Point(1, 2)));
    auto reactor = distinct(cell);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval().m_y == 2);
    cell->set(Point(1, 3));
    REQUIRE(reactor.commit(1) == State::NONE);
    cell->set(Point(2, 3));
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval().m_y == 3);
  }

  TEST_CASE("allocating_value") {
    auto cell = Shared(Cell(std::string("a")));
    auto reactor = distinct(cell);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == "a");
    cell->set("b");
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == "b");
    cell->set("a");
    REQUIRE(reactor.commit(2) == State::NONE);
  }

  TEST_CASE("completion_with_a_repeat") {
    auto queue = Shared(Queue<int>());
    auto reactor = distinct(queue);
    queue->push(10);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
    queue->set_complete(10);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("continuation_with_a_repeat") {
    auto queue = Shared(Queue<int>());
    auto reactor = distinct(queue);
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
    auto reactor = distinct(queue);
    queue->push(10);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }
}

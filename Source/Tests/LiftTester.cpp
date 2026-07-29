#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

namespace {
  int no_parameter_function() {
    return 512;
  }

  int no_parameter_function_throw() {
    throw std::runtime_error("");
  }

  int square(int x) {
    return x * x;
  }

  /** Counts how many times it is copied. */
  struct Counted {
    static inline auto copies = 0;

    int m_value;

    Counted()
      : m_value(0) {}

    explicit Counted(int value)
      : m_value(value) {}

    Counted(const Counted& counted)
        : m_value(counted.m_value) {
      ++copies;
    }

    Counted(Counted&& counted) noexcept
      : m_value(counted.m_value) {}

    Counted& operator =(const Counted& counted) {
      m_value = counted.m_value;
      ++copies;
      return *this;
    }

    Counted& operator =(Counted&& counted) noexcept {
      m_value = counted.m_value;
      return *this;
    }
  };

  /** A value that cannot be default constructed. */
  struct Required {
    int m_value;

    explicit Required(int value)
      : m_value(value) {}
  };
}

TEST_SUITE("Lift") {
  TEST_CASE("lift_no_parameters") {
    auto reactor = Lift(no_parameter_function);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 512);
  }

  TEST_CASE("lift_no_parameters_throw") {
    auto reactor = Lift(no_parameter_function_throw);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("lift_constant_argument") {
    auto reactor = Lift(square, Constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 25);
  }

  TEST_CASE("lift_one_argument_updates") {
    auto queue = Shared(Queue<int>());
    auto reactor = Lift(square, queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 100);
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(reactor.eval() == 100);
    queue->push(5);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 25);
    REQUIRE(reactor.commit(4) == State::NONE);
    REQUIRE(reactor.eval() == 25);
    queue->set_complete(4);
    REQUIRE(reactor.commit(5) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 16);
  }

  TEST_CASE("complete_arguments") {
    auto queue = Shared(Queue<int>());
    queue->push(10);
    queue.commit(0);
    queue->set_complete();
    auto reactor = Lift(square, queue);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 100);
  }

  TEST_CASE("lifting_over_several_arguments") {
    auto left = Shared(Queue<int>());
    auto right = Shared(Queue<int>());
    auto reactor = Lift([] (int a, int b) {
      return a * b;
    }, left, right);
    left->push(3);
    REQUIRE(reactor.commit(0) == State::NONE);
    right->push(4);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 12);
    left->push(5);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 20);
  }

  TEST_CASE("an_argument_that_throws") {
    auto queue = Shared(Queue<int>());
    auto reactor = Lift(square, queue);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("lifting_a_void_function") {
    auto queue = Shared(Queue<int>());
    auto total = std::make_shared<int>(0);
    auto reactor = Lift([total] (int value) {
      *total += value;
    }, queue);
    REQUIRE((std::is_same_v<decltype(reactor)::Type, void>));
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(3);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(*total == 3);
    queue->push(4);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(*total == 7);
  }

  TEST_CASE("lifting_a_void_function_that_throws") {
    auto queue = Shared(Queue<int>());
    auto reactor = Lift([] (int value) {
      throw std::runtime_error("fail");
    }, queue);
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("lifting_to_a_type_without_a_default") {
    auto cell = Shared(Cell(1));
    auto reactor = Lift([] (int value) noexcept {
      return Required(value);
    }, cell);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval().m_value == 1);
    cell->set(7);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval().m_value == 7);
  }

  TEST_CASE("an_evaluation_does_not_copy_its_arguments") {
    auto left = Shared(Cell(Counted(1)));
    auto right = Shared(Cell(Counted(2)));
    auto reactor = Lift([] (const Counted& a, const Counted& b) noexcept {
      return Counted(a.m_value + b.m_value);
    }, left, right);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval().m_value == 3);
    auto copies = Counted::copies;
    left->set(Counted(10));
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval().m_value == 12);
    REQUIRE(Counted::copies == copies);
  }

  TEST_CASE("lifting_a_function_returning_an_optional") {
    auto queue = Shared(Queue<int>());
    auto reactor = Lift([] (int value) {
      if(value % 2 == 0) {
        return std::optional<int>(value);
      }
      return std::optional<int>();
    }, queue);
    REQUIRE((std::is_same_v<decltype(reactor)::Type, int>));
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("lifting_a_function_returning_an_evaluation") {
    auto queue = Shared(Queue<int>());
    auto reactor = Lift([] (int value) {
      if(value < 0) {
        return FunctionEvaluation<int>(State::COMPLETE);
      }
      return FunctionEvaluation<int>(value);
    }, queue);
    queue->push(5);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    queue->push(-1);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("a_function_asking_to_be_committed_again") {
    auto queue = Shared(Queue<int>());
    auto count = std::make_shared<int>(0);
    auto reactor = Lift([count] (int value) {
      ++*count;
      if(*count < 3) {
        return FunctionEvaluation<int>(*count, State::CONTINUE);
      }
      return FunctionEvaluation<int>(*count);
    }, queue);
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(reactor.commit(3) == State::NONE);
  }

  TEST_CASE("an_evaluation_is_noexcept_when_everything_is") {
    auto cell = Shared(Cell(1));
    auto safe = Lift([] (int value) noexcept {
      return value;
    }, cell);
    REQUIRE(decltype(safe)::is_noexcept);
    auto throwing_function = Lift([] (int value) {
      return value;
    }, cell);
    REQUIRE(!decltype(throwing_function)::is_noexcept);
    auto queue = Shared(Queue<int>());
    auto throwing_argument = Lift([] (int value) noexcept {
      return value;
    }, queue);
    REQUIRE(!decltype(throwing_argument)::is_noexcept);
  }
}

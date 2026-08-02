#include <memory>
#include <stdexcept>
#include <string>
#include <doctest/doctest.h>
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Fold.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

namespace {
  struct Accumulator {
    auto operator ()(const auto& left, const auto& right) const {
      return left + right;
    }
  };
}

TEST_SUITE("Fold") {
  TEST_CASE("no_values") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto reactor = Fold(
      Lift(Accumulator(), left, right), left, right, none<int>());
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("single_value") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto reactor = Fold(
      Lift(Accumulator(), left, right), left, right, constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("two_values") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto reactor = Fold(Lift(Accumulator(), left, right), left, right,
      chain(constant(5), constant(10)));
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 15);
  }

  TEST_CASE("three_values") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto reactor = Fold(Lift(Accumulator(), left, right), left, right,
      chain(constant(5), chain(constant(10), constant(20))));
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 15);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 35);
  }

  TEST_CASE("make_fold") {
    auto reactor = fold(Accumulator(), chain(1, chain(2, 3)));
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 6);
  }

  TEST_CASE("evaluator_without_a_value") {
    auto blank = std::string();
    auto head = std::string(32, 'a');
    auto tail = std::string(32, 'b');
    auto left = make_fold_argument<std::string>();
    auto right = make_fold_argument<std::string>();
    auto reactor = Fold(Lift([] (const auto& left, const auto& right) {
      if(right->empty()) {
        return FunctionEvaluation<std::string>(State::NONE);
      }
      return FunctionEvaluation<std::string>(*left + *right);
    }, left, right), left, right, chain(head, chain(blank, tail)));
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::CONTINUE);
    REQUIRE(reactor.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == head + tail);
  }

  TEST_CASE("exception") {
    auto queue = Shared(Queue<int>());
    auto reactor = fold(Accumulator(), queue);
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("exception_before_a_value") {
    auto queue = Shared(Queue<int>());
    auto reactor = fold(Accumulator(), queue);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("continuing_evaluator") {
    auto queue = Shared(Queue<int>());
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto count = std::make_shared<int>(0);
    auto reactor = Fold(Lift([count] (const auto& left, const auto& right) {
      ++*count;
      if(*count == 1) {
        return FunctionEvaluation<int>(*left + *right, State::CONTINUE);
      }
      return FunctionEvaluation<int>(100);
    }, left, right), left, right, queue);
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(2);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 100);
  }

  TEST_CASE("completing_evaluator") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto queue = Shared(Queue<int>());
    auto reactor = Fold(Lift([] (const auto& left, const auto& right) {
      return FunctionEvaluation<int>(*left + *right, State::COMPLETE);
    }, left, right), left, right, queue);
    queue->push(1);
    queue->push(2);
    queue->push(3);
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
  }

  TEST_CASE("discarding_a_continuation_at_completion") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto count = std::make_shared<int>(0);
    auto reactor = Fold(Lift([count] (const auto& left, const auto& right) {
      ++*count;
      if(*count == 1) {
        return FunctionEvaluation<int>(State::CONTINUE);
      }
      return FunctionEvaluation<int>(*left + *right);
    }, left, right), left, right, chain(1, 2));
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("value_arriving_during_a_continuation") {
    auto queue = Shared(Queue<int>());
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto reactor = Fold(Lift([] (const auto& left, const auto& right) {
      return FunctionEvaluation<int>(*left + *right, State::CONTINUE);
    }, left, right), left, right, queue);
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(2);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    queue->push(10);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 13);
  }

  TEST_CASE("completion_with_a_continuation") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto reactor = Fold(Lift([] (const auto& left, const auto& right) {
      return FunctionEvaluation<int>(*left + *right, State::CONTINUE);
    }, left, right), left, right, chain(1, 2));
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
  }
}

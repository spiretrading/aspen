#include <memory>
#include <stdexcept>
#include <string>
#include <doctest/doctest.h>
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Fold.hpp"
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
    auto f = Fold(Lift(Accumulator(), left, right), left, right, none<int>());
    REQUIRE(f.commit(0) == State::COMPLETE);
  }

  TEST_CASE("single_value") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto f = Fold(Lift(Accumulator(), left, right), left, right, constant(5));
    REQUIRE(f.commit(0) == State::COMPLETE);
  }

  TEST_CASE("two_values") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto f = Fold(Lift(Accumulator(), left, right), left, right,
      chain(constant(5), constant(10)));
    REQUIRE(f.commit(0) == State::CONTINUE);
    REQUIRE(f.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(f.eval() == 15);
  }

  TEST_CASE("three_values") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto f = Fold(Lift(Accumulator(), left, right), left, right,
      chain(constant(5), chain(constant(10), constant(20))));
    REQUIRE(f.commit(0) == State::CONTINUE);
    REQUIRE(f.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(f.eval() == 15);
    REQUIRE(f.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(f.eval() == 35);
  }

  TEST_CASE("fold_function") {
    auto f = fold(Accumulator(), chain(1, chain(2, 3)));
    REQUIRE(f.commit(0) == State::CONTINUE);
    REQUIRE(f.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(f.eval() == 3);
    REQUIRE(f.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(f.eval() == 6);
  }

  TEST_CASE("evaluator_without_a_value") {
    auto blank = std::string();
    auto head = std::string(32, 'a');
    auto tail = std::string(32, 'b');
    auto left = make_fold_argument<std::string>();
    auto right = make_fold_argument<std::string>();
    auto f = Fold(Lift([] (const auto& left, const auto& right) {
      if(right->empty()) {
        return FunctionEvaluation<std::string>(State::NONE);
      }
      return FunctionEvaluation<std::string>(*left + *right);
    }, left, right), left, right, chain(head, chain(blank, tail)));
    REQUIRE(f.commit(0) == State::CONTINUE);
    REQUIRE(f.commit(1) == State::CONTINUE);
    REQUIRE(f.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(f.eval() == head + tail);
  }

  TEST_CASE("exception") {
    auto queue = Shared(Queue<int>());
    auto f = fold(Accumulator(), queue);
    queue->push(1);
    REQUIRE(f.commit(0) == State::NONE);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(f.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(f.eval(), std::runtime_error);
  }

  TEST_CASE("exception_before_a_value") {
    auto queue = Shared(Queue<int>());
    auto f = fold(Accumulator(), queue);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(f.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(f.eval(), std::runtime_error);
  }

  TEST_CASE("continuing_evaluator") {
    auto queue = Shared(Queue<int>());
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto count = std::make_shared<int>(0);
    auto f = Fold(Lift([count] (const auto& left, const auto& right) {
      ++*count;
      if(*count == 1) {
        return FunctionEvaluation<int>(*left + *right, State::CONTINUE);
      }
      return FunctionEvaluation<int>(100);
    }, left, right), left, right, queue);
    queue->push(1);
    REQUIRE(f.commit(0) == State::NONE);
    queue->push(2);
    REQUIRE(f.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(f.eval() == 3);
    REQUIRE(f.commit(2) == State::EVALUATED);
    REQUIRE(f.eval() == 100);
  }

  TEST_CASE("completing_evaluator") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto queue = Shared(Queue<int>());
    auto f = Fold(Lift([] (const auto& left, const auto& right) {
      return FunctionEvaluation<int>(*left + *right, State::COMPLETE);
    }, left, right), left, right, queue);
    queue->push(1);
    queue->push(2);
    queue->push(3);
    REQUIRE(f.commit(0) == State::CONTINUE);
    REQUIRE(f.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(f.eval() == 3);
  }

  TEST_CASE("deferred_final_evaluation") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto count = std::make_shared<int>(0);
    auto f = Fold(Lift([count] (const auto& left, const auto& right) {
      ++*count;
      if(*count == 1) {
        return FunctionEvaluation<int>(State::CONTINUE);
      }
      return FunctionEvaluation<int>(*left + *right);
    }, left, right), left, right, chain(1, 2));
    REQUIRE(f.commit(0) == State::CONTINUE);
    REQUIRE(f.commit(1) == State::COMPLETE);
  }

  TEST_CASE("value_arriving_during_a_continuation") {
    auto queue = Shared(Queue<int>());
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto f = Fold(Lift([] (const auto& left, const auto& right) {
      return FunctionEvaluation<int>(*left + *right, State::CONTINUE);
    }, left, right), left, right, queue);
    queue->push(1);
    REQUIRE(f.commit(0) == State::NONE);
    queue->push(2);
    REQUIRE(f.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(f.eval() == 3);
    queue->push(10);
    REQUIRE(f.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(f.eval() == 13);
  }

  TEST_CASE("completion_with_a_continuation") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto f = Fold(Lift([] (const auto& left, const auto& right) {
      return FunctionEvaluation<int>(*left + *right, State::CONTINUE);
    }, left, right), left, right, chain(1, 2));
    REQUIRE(f.commit(0) == State::CONTINUE);
    REQUIRE(f.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(f.eval() == 3);
  }
}

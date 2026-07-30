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
  TEST_CASE("fold_empty") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto f = Fold(Lift(Accumulator(), left, right), left, right, none<int>());
    REQUIRE(f.commit(0) == State::COMPLETE);
  }

  TEST_CASE("fold_single_value") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto f = Fold(Lift(Accumulator(), left, right), left, right, constant(5));
    REQUIRE(f.commit(0) == State::COMPLETE);
  }

  TEST_CASE("fold_two_values") {
    auto left = make_fold_argument<int>();
    auto right = make_fold_argument<int>();
    auto f = Fold(Lift(Accumulator(), left, right), left, right,
      chain(constant(5), constant(10)));
    REQUIRE(f.commit(0) == State::CONTINUE);
    REQUIRE(f.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(f.eval() == 15);
  }

  TEST_CASE("fold_three_values") {
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

  TEST_CASE("fold_through_the_free_function") {
    auto f = fold(Accumulator(), chain(1, chain(2, 3)));
    REQUIRE(f.commit(0) == State::CONTINUE);
    REQUIRE(f.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(f.eval() == 3);
    REQUIRE(f.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(f.eval() == 6);
  }

  TEST_CASE("fold_keeps_its_value_when_the_evaluator_does_not_evaluate") {
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

  TEST_CASE("fold_propagates_an_exception") {
    auto queue = Shared(Queue<int>());
    auto f = fold(Accumulator(), queue);
    queue->push(1);
    REQUIRE(f.commit(0) == State::NONE);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(f.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(f.eval(), std::runtime_error);
  }

  TEST_CASE("fold_does_not_continue_when_its_evaluator_completes") {
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

  TEST_CASE("fold_does_not_complete_with_a_continuation") {
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

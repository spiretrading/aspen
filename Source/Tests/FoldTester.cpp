#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

namespace {
  struct Accumulator {
    template<typename T>
    auto operator ()(const T& a, const T& b) {
      return a + b;
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
}

#include <catch2/catch.hpp>
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Fold.hpp"

using namespace Aspen;

namespace {
  struct Accumulator {
    template<typename T>
    auto operator ()(const T& a, const T& b) {
      return a + b;
    }
  };
}

TEST_CASE("test_fold_empty", "[Fold]") {
  auto left = make_fold_argument<int>();
  auto right = make_fold_argument<int>();
  auto f = Fold(Lift(Accumulator(), left, right), left, right, none<int>());
  REQUIRE(f.commit(0) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_fold_single_value", "[Fold]") {
  auto left = make_fold_argument<int>();
  auto right = make_fold_argument<int>();
  auto f = Fold(Lift(Accumulator(), left, right), left, right, constant(5));
  REQUIRE(f.commit(0) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_fold_two_values", "[Fold]") {
  auto left = make_fold_argument<int>();
  auto right = make_fold_argument<int>();
  auto f = Fold(Lift(Accumulator(), left, right), left, right,
    chain(constant(5), constant(10)));
  REQUIRE(f.commit(0) == State::CONTINUE_EMPTY);
  REQUIRE(f.commit(1) == State::COMPLETE_EVALUATED);
  REQUIRE(f.eval() == 15);
}

TEST_CASE("test_fold_three_values", "[Fold]") {
  auto left = make_fold_argument<int>();
  auto right = make_fold_argument<int>();
  auto f = Fold(Lift(Accumulator(), left, right), left, right,
    chain(constant(5), chain(constant(10), constant(20))));
  REQUIRE(f.commit(0) == State::CONTINUE_EMPTY);
  REQUIRE(f.commit(1) == State::CONTINUE_EVALUATED);
  REQUIRE(f.eval() == 15);
  REQUIRE(f.commit(2) == State::COMPLETE_EVALUATED);
  REQUIRE(f.eval() == 35);
}

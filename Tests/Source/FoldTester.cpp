#include <catch2/catch.hpp>
#include "Aspen/None.hpp"
#include "Aspen/Fold.hpp"

using namespace Aspen;

TEST_CASE("test_fold_empty", "[Fold]") {
  auto left = make_fold_argument<int>();
  auto right = make_fold_argument<int>();
  auto f = Fold(Lift([] (auto a, auto b) { return 0; }, left, right), left,
    right, none<int>());
}

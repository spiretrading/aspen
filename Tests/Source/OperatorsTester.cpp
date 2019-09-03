#include <catch2/catch.hpp>
#include "Aspen/Constant.hpp"
#include "Aspen/Operators.hpp"

using namespace Aspen;

namespace {
  template<typename T, typename U>
  void require(T&& reactor, const U& value) {
    auto state = reactor.commit(0);
    REQUIRE(has_evaluation(state));
    REQUIRE(reactor.eval() == value);
  }
}

TEST_CASE("test_operators", "[Operators]") {
  require(constant(10) + Constant<int>(20), 30);
  require(constant(10) - Constant<int>(20), -10);
  require(constant(10) * Constant<int>(20), 200);
  require(constant(3.5) / Constant<double>(0.5), 7.0);
  require(constant(10) % Constant<int>(3), 1);
  require(constant(4) ^ Constant<int>(6), 2);
  require(Constant<int>(4) & Constant<int>(6), 4);
  require(Constant<int>(4) | Constant<int>(1), 5);
  require(~Constant<int>(4), ~4);
  require(Constant<int>(4) << constant(2), 16);
  require(Constant<int>(32) >> constant(3), 4);
  require(!Constant<bool>(true), false);
  require(!Constant<bool>(false), true);
  require(Constant<int>(5) < constant(10), true);
  require(Constant<int>(15) < constant(10), false);
  require(Constant<int>(5) <= constant(10), true);
  require(Constant<int>(5) <= constant(5), true);
  require(Constant<int>(7) <= constant(5), false);
  require(Constant<int>(5) == constant(5), true);
  require(Constant<int>(5) == constant(6), false);
  require(Constant<int>(5) != constant(5), false);
  require(Constant<int>(5) != constant(6), true);
  require(Constant<int>(12) >= constant(10), true);
  require(Constant<int>(12) >= constant(12), true);
  require(Constant<int>(7) >= constant(12), false);
  require(Constant<int>(5) > constant(10), false);
  require(Constant<int>(15) > constant(10), true);
  require(-constant(10), -10);
  require(+constant(10), +10);
  require(Constant<bool>(true) && constant(true), true);
  require(Constant<bool>(false) && constant(true), false);
  require(Constant<bool>(true) || constant(false), true);
  require(Constant<bool>(false) || constant(false), false);
}

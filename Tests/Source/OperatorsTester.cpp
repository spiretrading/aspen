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
  require(constant(10) + std::make_unique<Constant<int>>(20), 30);
  require(constant(10) - std::make_unique<Constant<int>>(20), -10);
  require(constant(10) * std::make_unique<Constant<int>>(20), 200);
  require(constant(3.5) / std::make_unique<Constant<double>>(0.5), 7.0);
  require(constant(10) % std::make_shared<Constant<int>>(3), 1);
  require(constant(4) ^ std::make_shared<Constant<int>>(6), 2);
  require(std::make_unique<Constant<int>>(4) &
    std::make_shared<Constant<int>>(6), 4);
  require(std::make_unique<Constant<int>>(4) |
    std::make_shared<Constant<int>>(1), 5);
  require(~std::make_unique<Constant<int>>(4), ~4);
  require(std::make_unique<Constant<int>>(4) << constant(2), 16);
  require(std::make_unique<Constant<int>>(32) >> constant(3), 4);
  require(!std::make_unique<Constant<bool>>(true), false);
  require(!std::make_unique<Constant<bool>>(false), true);
  require(std::make_unique<Constant<int>>(5) < constant(10), true);
  require(std::make_unique<Constant<int>>(15) < constant(10), false);
  require(std::make_unique<Constant<int>>(5) <= constant(10), true);
  require(std::make_unique<Constant<int>>(5) <= constant(5), true);
  require(std::make_unique<Constant<int>>(7) <= constant(5), false);
  require(std::make_unique<Constant<int>>(5) == constant(5), true);
  require(std::make_unique<Constant<int>>(5) == constant(6), false);
  require(std::make_unique<Constant<int>>(5) != constant(5), false);
  require(std::make_unique<Constant<int>>(5) != constant(6), true);
  require(std::make_unique<Constant<int>>(12) >= constant(10), true);
  require(std::make_unique<Constant<int>>(12) >= constant(12), true);
  require(std::make_unique<Constant<int>>(7) >= constant(12), false);
  require(std::make_unique<Constant<int>>(5) > constant(10), false);
  require(std::make_unique<Constant<int>>(15) > constant(10), true);
  require(-constant(10), -10);
  require(+constant(10), +10);
  require(std::make_unique<Constant<bool>>(true) && constant(true), true);
  require(std::make_unique<Constant<bool>>(false) && constant(true), false);
  require(std::make_unique<Constant<bool>>(true) || constant(false), true);
  require(std::make_unique<Constant<bool>>(false) || constant(false), false);
}

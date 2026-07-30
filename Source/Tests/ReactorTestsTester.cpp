#include <type_traits>
#include <doctest/doctest.h>
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

static_assert(IsReactor<ByValueReactor<CountedValue>>);
static_assert(std::is_same_v<decltype(
  std::declval<const ByValueReactor<CountedValue>&>().eval()), CountedValue>);

TEST_SUITE("ReactorTests") {
  TEST_CASE("counting_copies_and_destructions") {
    CountedValue::reset_counts();
    {
      auto value = CountedValue(5);
      REQUIRE(value.get_value() == 5);
      REQUIRE(CountedValue::get_destructions() == 0);
      auto copy = value;
      REQUIRE(copy.get_value() == 5);
      REQUIRE(CountedValue::get_copies() == 1);
      REQUIRE(CountedValue::get_destructions() == 0);
    }
    REQUIRE(CountedValue::get_destructions() == 2);
  }

  TEST_CASE("evaluating_a_reactor_by_value") {
    auto reactor = ByValueReactor(CountedValue(7));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval().get_value() == 7);
    CountedValue::reset_counts();
    [[maybe_unused]] const auto& value = reactor.eval();
    REQUIRE(CountedValue::get_copies() == 1);
  }

  TEST_CASE("an_evaluation_that_outlives_its_reactor_call") {
    auto reactor = ByValueReactor(CountedValue(9));
    test_evaluation_lifetime(reactor);
  }
}

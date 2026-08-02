#include <type_traits>
#include <doctest/doctest.h>
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

static_assert(IsReactor<ByValueReactor<CountedValue>>);
static_assert(std::is_same_v<decltype(
  std::declval<const ByValueReactor<CountedValue>&>().eval()), CountedValue>);

TEST_SUITE("ReactorTests") {
  TEST_CASE("counted_value") {
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

  TEST_CASE("by_value_reactor") {
    auto reactor = ByValueReactor(CountedValue(7));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval().get_value() == 7);
    CountedValue::reset_counts();
    [[maybe_unused]] const auto& value = reactor.eval();
    REQUIRE(CountedValue::get_copies() == 1);
  }

  TEST_CASE("tracked_reactor") {
    auto token = std::weak_ptr<void>();
    {
      auto reactor = TrackedReactor<int>(5, State::COMPLETE_EVALUATED);
      token = reactor.get_token();
      REQUIRE(!token.expired());
      REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
      REQUIRE(reactor.eval() == 5);
      auto moved = std::move(reactor);
      REQUIRE(!token.expired());
    }
    REQUIRE(token.expired());
  }

  TEST_CASE("counting_reactor") {
    auto reactor = CountingReactor<int>(7, State::CONTINUE_EVALUATED);
    REQUIRE(reactor.get_commits() == 0);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.get_commits() == 1);
    REQUIRE(reactor.eval() == 7);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.get_commits() == 2);
  }

  TEST_CASE("evaluation_lifetime") {
    auto reactor = ByValueReactor(CountedValue(9));
    test_by_value_evaluation(reactor, 9);
  }
}

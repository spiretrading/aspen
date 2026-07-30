#include <cstdint>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/Perpetual.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Tests/ReactorTests.hpp"
#include "Aspen/Traits.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

namespace {
  struct Fragile {
    Fragile() = default;
    Fragile(const Fragile& fragile) {}
  };

  struct FragileReactor {
    using Type = Fragile;

    State commit(std::uint64_t sequence) noexcept {
      return State::COMPLETE_EVALUATED;
    }

    Fragile eval() const noexcept {
      return Fragile();
    }
  };

  struct ValueReactor {
    using Type = int;

    State commit(std::uint64_t sequence) noexcept {
      return State::COMPLETE_EVALUATED;
    }

    int eval() const noexcept {
      return 1;
    }
  };
}

TEST_SUITE("Traits") {
  TEST_CASE("wrapping_a_type_into_a_reactor") {
    REQUIRE((std::is_same_v<to_reactor_t<int>, Constant<int>>));
    REQUIRE((std::is_same_v<to_reactor_t<Constant<int>>, Constant<int>>));
    REQUIRE((std::is_same_v<to_reactor_t<const Constant<int>&>, Constant<int>>));
  }

  TEST_CASE("determining_what_a_reactor_evaluates_to") {
    REQUIRE((std::is_same_v<reactor_result_t<int>, int>));
    REQUIRE((std::is_same_v<reactor_result_t<Constant<int>>, int>));
    REQUIRE((std::is_same_v<reactor_result_t<Perpetual>, void>));
    REQUIRE((std::is_same_v<eval_result_t<int>, const int&>));
    REQUIRE((std::is_same_v<eval_result_t<void>, void>));
  }

  TEST_CASE("determining_how_an_evaluation_is_returned") {
    REQUIRE((std::is_same_v<reactor_evaluation_t<Constant<int>>, const int&>));
    REQUIRE((std::is_same_v<reactor_evaluation_t<ByValueReactor<int>>, int>));
    REQUIRE(is_reference_evaluation_v<Constant<int>>);
    REQUIRE(is_reference_evaluation_v<Queue<int>>);
    REQUIRE(!is_reference_evaluation_v<ByValueReactor<int>>);
    REQUIRE(!is_reference_evaluation_v<Perpetual>);
  }

  TEST_CASE("combining_the_evaluations_of_several_reactors") {
    REQUIRE((std::is_same_v<common_result_t<Constant<int>>, int>));
    REQUIRE((std::is_same_v<
      common_result_t<Constant<int>, ByValueReactor<int>>, int>));
    REQUIRE((std::is_same_v<common_evaluation_t<Constant<int>>, const int&>));
    REQUIRE((std::is_same_v<
      common_evaluation_t<Constant<int>, Queue<int>>, const int&>));
    REQUIRE((std::is_same_v<common_evaluation_t<ByValueReactor<int>>, int>));
    REQUIRE((std::is_same_v<
      common_evaluation_t<Constant<int>, ByValueReactor<int>>, int>));
    REQUIRE((std::is_same_v<common_evaluation_t<Perpetual>, void>));
  }

  TEST_CASE("determining_whether_an_evaluation_is_noexcept") {
    REQUIRE(is_noexcept_reactor_v<Constant<int>>);
    REQUIRE(!is_noexcept_reactor_v<Queue<int>>);
    REQUIRE(is_noexcept_reactor_v<Constant<int>, Cell<int>>);
    REQUIRE(!is_noexcept_reactor_v<Constant<int>, Queue<int>>);
    REQUIRE(is_noexcept_evaluation_v<Constant<int>>);
    REQUIRE(is_noexcept_evaluation_v<Perpetual>);
    REQUIRE(!is_noexcept_evaluation_v<Queue<int>>);
    REQUIRE(!is_noexcept_evaluation_v<Constant<int>, Queue<int>>);
  }

  TEST_CASE("evaluating_by_value_decides_whether_a_copy_can_throw") {
    REQUIRE(is_noexcept_reactor_v<ValueReactor>);
    REQUIRE(is_noexcept_evaluation_v<ValueReactor>);
    REQUIRE(is_noexcept_reactor_v<FragileReactor>);
    REQUIRE(!is_noexcept_evaluation_v<FragileReactor>);
    REQUIRE((std::is_same_v<common_evaluation_t<FragileReactor>, Fragile>));
  }

  TEST_CASE("applying_a_function_to_every_element") {
    auto tuple = std::tuple(1, 2, 3);
    auto total = 0;
    for_each(tuple, [&] (const auto& element) {
      total += element;
    });
    REQUIRE(total == 6);
    for_each(tuple, [] (auto& element) {
      element *= 2;
    });
    REQUIRE(std::get<0>(tuple) == 2);
    REQUIRE(std::get<2>(tuple) == 6);
  }

  TEST_CASE("assigning_an_evaluation") {
    auto reactor = Constant(5);
    reactor.commit(0);
    auto value = std::optional<Maybe<int>>();
    try_assign(value, reactor);
    REQUIRE(**value == 5);
  }

  TEST_CASE("assigning_an_evaluation_that_throws") {
    auto reactor = Queue<int>();
    reactor.set_complete(std::runtime_error("fail"));
    reactor.commit(0);
    auto value = Maybe<int>();
    try_assign(value, reactor);
    REQUIRE(value.has_exception());
    REQUIRE_THROWS_AS(value.get(), std::runtime_error);
  }

  TEST_CASE("assigning_an_evaluation_of_nothing") {
    auto reactor = Perpetual();
    reactor.commit(0);
    auto value = Maybe<void>();
    try_assign(value, reactor);
    REQUIRE(!value.has_exception());
  }
}

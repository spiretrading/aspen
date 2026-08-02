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

  struct ThrowingVoid {
    using Type = void;

    State commit(std::uint64_t sequence) noexcept {
      return State::EVALUATED;
    }

    void eval() const {
      throw std::runtime_error("fail");
    }
  };

  template<typename R>
  concept HasEvaluation = requires {
    typename reactor_evaluation_t<R>;
  };
}

TEST_SUITE("Traits") {
  TEST_CASE("to_reactor_trait") {
    REQUIRE((std::is_same_v<to_reactor_t<int>, Constant<int>>));
    REQUIRE((std::is_same_v<to_reactor_t<Constant<int>>, Constant<int>>));
    REQUIRE((std::is_same_v<
      to_reactor_t<const Constant<int>&>, Constant<int>>));
  }

  TEST_CASE("reactor_result_trait") {
    REQUIRE((std::is_same_v<reactor_result_t<int>, int>));
    REQUIRE((std::is_same_v<reactor_result_t<Constant<int>>, int>));
    REQUIRE((std::is_same_v<reactor_result_t<Perpetual>, void>));
  }

  TEST_CASE("eval_result_trait") {
    REQUIRE((std::is_same_v<eval_result_t<int>, const int&>));
    REQUIRE((std::is_same_v<eval_result_t<void>, void>));
  }

  TEST_CASE("evaluation_trait") {
    REQUIRE((std::is_same_v<reactor_evaluation_t<Constant<int>>, const int&>));
    REQUIRE((std::is_same_v<reactor_evaluation_t<ByValueReactor<int>>, int>));
    REQUIRE(is_reference_evaluation_v<Constant<int>>);
    REQUIRE(is_reference_evaluation_v<Queue<int>>);
    REQUIRE(!is_reference_evaluation_v<ByValueReactor<int>>);
    REQUIRE(!is_reference_evaluation_v<Perpetual>);
  }

  TEST_CASE("evaluation_of_a_qualified_type") {
    static_assert(HasEvaluation<Constant<int>>);
    static_assert(!HasEvaluation<Constant<int>&>);
    static_assert(!HasEvaluation<const Constant<int>&>);
    static_assert(!HasEvaluation<const Constant<int>>);
    static_assert(!HasEvaluation<int>);
  }

  TEST_CASE("common_evaluation_trait") {
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
    REQUIRE((std::is_same_v<
      common_evaluation_t<Constant<int>, Constant<double>>, double>));
  }

  TEST_CASE("noexcept_evaluation_trait") {
    REQUIRE(is_noexcept_reactor_v<Constant<int>>);
    REQUIRE(!is_noexcept_reactor_v<Queue<int>>);
    REQUIRE(is_noexcept_reactor_v<Constant<int>, Cell<int>>);
    REQUIRE(!is_noexcept_reactor_v<Constant<int>, Queue<int>>);
    REQUIRE(is_noexcept_evaluation_v<Constant<int>>);
    REQUIRE(is_noexcept_evaluation_v<Perpetual>);
    REQUIRE(!is_noexcept_evaluation_v<Queue<int>>);
    REQUIRE(!is_noexcept_evaluation_v<Constant<int>, Queue<int>>);
  }

  TEST_CASE("noexcept_evaluation_by_value") {
    REQUIRE(is_noexcept_reactor_v<ValueReactor>);
    REQUIRE(is_noexcept_evaluation_v<ValueReactor>);
    REQUIRE(is_noexcept_reactor_v<FragileReactor>);
    REQUIRE(!is_noexcept_evaluation_v<FragileReactor>);
    REQUIRE((std::is_same_v<common_evaluation_t<FragileReactor>, Fragile>));
  }

  TEST_CASE("for_each_function") {
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

  TEST_CASE("try_assign_function") {
    auto reactor = Constant(5);
    reactor.commit(0);
    auto value = std::optional<Maybe<int>>();
    try_assign(value, reactor);
    REQUIRE(**value == 5);
  }

  TEST_CASE("try_assign_an_exception") {
    auto reactor = Queue<int>();
    reactor.set_complete(std::runtime_error("fail"));
    reactor.commit(0);
    auto value = Maybe<int>();
    try_assign(value, reactor);
    REQUIRE(value.has_exception());
    REQUIRE_THROWS_AS(value.get(), std::runtime_error);
  }

  TEST_CASE("try_assign_void") {
    auto reactor = Perpetual();
    reactor.commit(0);
    auto value = Maybe<void>(std::make_exception_ptr(std::runtime_error("x")));
    REQUIRE(value.has_exception());
    try_assign(value, reactor);
    REQUIRE(!value.has_exception());
  }

  TEST_CASE("try_assign_a_throwing_void") {
    auto reactor = ThrowingVoid();
    reactor.commit(0);
    auto value = Maybe<void>();
    try_assign(value, reactor);
    REQUIRE(value.has_exception());
    REQUIRE_THROWS_AS(value.get(), std::runtime_error);
  }
}

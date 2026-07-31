#include <string>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Branch.hpp"
#include "Aspen/CommitHandler.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Perpetual.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/StateReactor.hpp"

using namespace Aspen;

namespace {
  struct Commiter {
    State commit(std::uint64_t sequence) noexcept {
      return State::NONE;
    }
  };

  struct Mismatched {
    using Type = int;

    State commit(std::uint64_t sequence) noexcept {
      return State::NONE;
    }

    double eval() const noexcept {
      return 0;
    }
  };

  struct Throwing {
    using Type = int;

    State commit(std::uint64_t sequence) {
      return State::NONE;
    }

    int eval() const noexcept {
      return 0;
    }
  };

  struct Mutating {
    using Type = int;

    State commit(std::uint64_t sequence) noexcept {
      return State::NONE;
    }

    int eval() noexcept {
      return 0;
    }
  };
}

TEST_SUITE("Reactor") {
  TEST_CASE("is_reactor") {
    REQUIRE(IsReactor<Constant<int>>);
    REQUIRE(IsReactor<None<int>>);
    REQUIRE(IsReactor<Perpetual>);
    REQUIRE(IsReactor<Box<int>>);
    REQUIRE(IsReactor<Shared<Constant<int>>>);
    REQUIRE(IsReactor<StateReactor<Constant<int>>>);
    REQUIRE(!IsReactor<int>);
    REQUIRE(!IsReactor<std::string>);
  }

  TEST_CASE("type_without_an_evaluation") {
    REQUIRE(!IsReactor<Commiter>);
    REQUIRE(!IsReactor<Branch<Constant<int>>>);
    REQUIRE(!IsReactor<CommitHandler<Constant<int>>>);
  }

  TEST_CASE("is_reactor_of") {
    REQUIRE((IsReactorOf<Constant<int>, int>));
    REQUIRE((IsReactorOf<None<std::string>, std::string>));
    REQUIRE((IsReactorOf<Box<int>, int>));
    REQUIRE((IsReactorOf<Shared<Constant<int>>, int>));
    REQUIRE(!(IsReactorOf<Constant<int>, double>));
    REQUIRE(!(IsReactorOf<Constant<int>, void>));
    REQUIRE(!(IsReactorOf<int, int>));
  }

  TEST_CASE("void_reactor") {
    REQUIRE((IsReactorOf<Perpetual, void>));
    REQUIRE((IsReactorOf<Box<void>, void>));
    REQUIRE(!(IsReactorOf<Perpetual, int>));
  }

  TEST_CASE("by_value_reactor") {
    REQUIRE((IsReactorOf<StateReactor<Constant<int>>, State>));
    REQUIRE(!(IsReactorOf<StateReactor<Constant<int>>, int>));
  }

  TEST_CASE("type_with_a_throwing_commit") {
    REQUIRE(!IsReactor<Throwing>);
    REQUIRE(!(IsReactorOf<Throwing, int>));
  }

  TEST_CASE("type_with_a_mismatched_evaluation") {
    REQUIRE(!IsReactor<Mismatched>);
    REQUIRE(!(IsReactorOf<Mismatched, int>));
    REQUIRE(!(IsReactorOf<Mismatched, double>));
  }

  TEST_CASE("type_with_a_non_const_evaluation") {
    REQUIRE(!IsReactor<Mutating>);
    REQUIRE(!(IsReactorOf<Mutating, int>));
  }

  TEST_CASE("qualified_reactor_type") {
    REQUIRE(!IsReactor<Constant<int>&>);
    REQUIRE(!IsReactor<const Constant<int>&>);
    REQUIRE(!IsReactor<const Constant<int>>);
    REQUIRE(!IsReactor<Constant<int>&&>);
  }
}

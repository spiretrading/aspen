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
}

TEST_SUITE("Reactor") {
  TEST_CASE("identifying_a_reactor") {
    REQUIRE(IsReactor<Constant<int>>);
    REQUIRE(IsReactor<None<int>>);
    REQUIRE(IsReactor<Perpetual>);
    REQUIRE(IsReactor<Box<int>>);
    REQUIRE(IsReactor<Shared<Constant<int>>>);
    REQUIRE(IsReactor<StateReactor<Constant<int>>>);
    REQUIRE(!IsReactor<int>);
    REQUIRE(!IsReactor<std::string>);
  }

  TEST_CASE("a_type_that_only_commits_is_not_a_reactor") {
    REQUIRE(!IsReactor<Commiter>);
    REQUIRE(!IsReactor<Branch<Constant<int>>>);
    REQUIRE(!IsReactor<CommitHandler<Constant<int>>>);
  }

  TEST_CASE("identifying_what_a_reactor_evaluates_to") {
    REQUIRE((IsReactorOf<Constant<int>, int>));
    REQUIRE((IsReactorOf<None<std::string>, std::string>));
    REQUIRE((IsReactorOf<Box<int>, int>));
    REQUIRE((IsReactorOf<Shared<Constant<int>>, int>));
    REQUIRE(!(IsReactorOf<Constant<int>, double>));
    REQUIRE(!(IsReactorOf<Constant<int>, void>));
    REQUIRE(!(IsReactorOf<int, int>));
  }

  TEST_CASE("a_reactor_evaluating_to_nothing") {
    REQUIRE((IsReactorOf<Perpetual, void>));
    REQUIRE((IsReactorOf<Box<void>, void>));
    REQUIRE(!(IsReactorOf<Perpetual, int>));
  }

  TEST_CASE("a_reactor_evaluating_by_value") {
    REQUIRE((IsReactorOf<StateReactor<Constant<int>>, State>));
    REQUIRE(!(IsReactorOf<StateReactor<Constant<int>>, int>));
  }

  TEST_CASE("a_type_disagreeing_with_its_own_type_is_not_a_reactor") {
    REQUIRE(!IsReactor<Mismatched>);
    REQUIRE(!(IsReactorOf<Mismatched, int>));
    REQUIRE(!(IsReactorOf<Mismatched, double>));
  }
}

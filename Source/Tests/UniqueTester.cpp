#include <memory>
#include <stdexcept>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Throw.hpp"
#include "Aspen/Unique.hpp"

using namespace Aspen;

TEST_SUITE("Unique") {
  TEST_CASE("unique_constant") {
    static_assert(IsReactorOf<Unique<Constant<int>>, int>);
    auto reactor = Unique(new Constant(123));
    REQUIRE(is_noexcept_reactor_v<decltype(reactor)>);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE((*reactor).eval() == 123);
    REQUIRE(reactor->eval() == 123);
    const auto& reference = reactor;
    REQUIRE((*reference).eval() == 123);
    REQUIRE(reference->eval() == 123);
  }

  TEST_CASE("unique_from_a_unique_ptr") {
    auto reactor = Unique(std::make_unique<Constant<int>>(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
  }

  TEST_CASE("moving_a_unique") {
    auto reactor = Unique(new Constant(7));
    auto moved = std::move(reactor);
    REQUIRE(moved.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(moved.eval() == 7);
  }

  TEST_CASE("assigning_into_an_uninitialized_unique") {
    auto reactor = Unique<Constant<int>>();
    reactor = Unique(new Constant(9));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 9);
  }

  TEST_CASE("a_reactor_that_fails") {
    auto reactor = Unique(new Throw<int>(std::runtime_error("fail")));
    REQUIRE(!is_noexcept_reactor_v<decltype(reactor)>);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }
}

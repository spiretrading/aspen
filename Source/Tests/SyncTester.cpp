#include <stdexcept>
#include <type_traits>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Sync.hpp"
#include "Aspen/Throw.hpp"

using namespace Aspen;

TEST_SUITE("Sync") {
  TEST_CASE("empty_sync") {
    auto value = 0;
    auto reactor = Sync(value, none<int>());
    REQUIRE(reactor.commit(0) == State::COMPLETE);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(value == 0);
  }

  TEST_CASE("single_sync") {
    auto value = 0;
    auto reactor = Sync(value, constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(value == 5);
  }

  TEST_CASE("exception_sync") {
    auto value = 0;
    auto reactor = Sync(value, chain(throws<int>(std::runtime_error("fail")),
      constant(12)));
    REQUIRE(!decltype(reactor)::is_noexcept);
    REQUIRE(!reactor.get_exception());
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(value == 0);
    REQUIRE(reactor.get_exception());
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(!reactor.get_exception());
    REQUIRE(reactor.eval() == 12);
    REQUIRE(value == 12);
  }

  TEST_CASE("sync_a_noexcept_reactor") {
    auto value = 0;
    auto cell = Shared(Cell(1));
    auto reactor = Sync(value, cell);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(!reactor.get_exception());
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(value == 1);
    cell->set(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(value == 2);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("sync_a_converted_value") {
    auto value = 0.0;
    auto reactor = Sync(value, constant(5));
    REQUIRE((std::is_same_v<decltype(reactor)::Type, double>));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(value == 5.0);
    REQUIRE(reactor.eval() == 5.0);
  }
}

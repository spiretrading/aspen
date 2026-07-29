#include <stdexcept>
#include <string>
#include <tuple>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/MultiSync.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Sync.hpp"
#include "Aspen/Throw.hpp"

using namespace Aspen;

TEST_SUITE("MultiSync") {
  TEST_CASE("empty_multi_sync") {
    auto record = std::tuple<int, std::string, double>();
    auto reactor = MultiSync(record, Sync(std::get<0>(record), none<int>()),
      Sync(std::get<1>(record), constant("hello")),
      Sync(std::get<2>(record), constant(3.14)));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
    REQUIRE(reactor.eval() == std::tuple(0, "", 0.0));
    REQUIRE(record == std::tuple(0, "", 0.0));
  }

  TEST_CASE("single_multi_sync") {
    auto record = std::tuple<int, std::string, double>();
    auto reactor = MultiSync(record, Sync(std::get<0>(record), constant(5)),
      Sync(std::get<1>(record), constant("hello")),
      Sync(std::get<2>(record), constant(3.14)));
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == std::tuple(5, "hello", 3.14));
    REQUIRE(record == std::tuple(5, "hello", 3.14));
  }

  TEST_CASE("exception_multi_sync") {
    auto record = std::tuple<int, std::string, double>();
    auto reactor = MultiSync(record, Sync(std::get<0>(record),
      chain(throws<int>(std::runtime_error("fail")), constant(12))),
      Sync(std::get<1>(record), constant("hello")),
      Sync(std::get<2>(record), constant(3.14)));
    REQUIRE(!decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == std::tuple(12, "hello", 3.14));
    REQUIRE(record == std::tuple(12, "hello", 3.14));
  }

  TEST_CASE("multi_sync_follows_every_update") {
    auto record = std::tuple<int, int>();
    auto left = Shared(Cell(1));
    auto right = Shared(Cell(2));
    auto reactor = MultiSync(record, Sync(std::get<0>(record), left),
      Sync(std::get<1>(record), right));
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(record == std::tuple(1, 2));
    left->set(10);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(record == std::tuple(10, 2));
    right->set(20);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(record == std::tuple(10, 20));
    REQUIRE(reactor.eval() == std::tuple(10, 20));
  }
}

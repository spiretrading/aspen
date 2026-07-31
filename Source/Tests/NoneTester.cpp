#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/None.hpp"
#include "Aspen/Reactor.hpp"

using namespace Aspen;

TEST_SUITE("None") {
  TEST_CASE("evaluation") {
    static_assert(IsReactorOf<None<int>, int>);
    static_assert(None<int>().commit(0) == State::COMPLETE);
    auto reactor = None<int>();
    REQUIRE(!is_noexcept_reactor_v<None<int>>);
    REQUIRE(reactor.commit(0) == State::COMPLETE);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("void_evaluation") {
    auto reactor = none<void>();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }
}

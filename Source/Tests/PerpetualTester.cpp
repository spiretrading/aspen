#include <doctest/doctest.h>
#include "Aspen/Perpetual.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Traits.hpp"

using namespace Aspen;

TEST_SUITE("Perpetual") {
  TEST_CASE("perpetual") {
    static_assert(IsReactorOf<Perpetual, void>);
    static_assert(Perpetual().commit(0) == State::CONTINUE_EVALUATED);
    auto reactor = Perpetual();
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(100) == State::CONTINUE_EVALUATED);
  }

  TEST_CASE("perpetual_function") {
    auto reactor = perpetual();
    REQUIRE(is_noexcept_reactor_v<decltype(reactor)>);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    reactor.eval();
  }
}

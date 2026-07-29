#include <exception>
#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Reactor.hpp"
#include "Aspen/Throw.hpp"

using namespace Aspen;

TEST_SUITE("Throw") {
  TEST_CASE("throw") {
    static_assert(IsReactorOf<Throw<int>, int>);
    auto reactor = Throw<int>(std::runtime_error(""));
    REQUIRE(!is_noexcept_reactor_v<Throw<int>>);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("throw_from_an_exception_ptr") {
    auto reactor = Throw<int>(
      std::make_exception_ptr(std::runtime_error("fail")));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_WITH_AS(reactor.eval(), "fail", std::runtime_error);
  }

  TEST_CASE("throw_nothing") {
    auto reactor = Throw<void>(std::logic_error("void"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::logic_error);
  }

  TEST_CASE("the_throws_function") {
    auto reactor = throws<int>(std::runtime_error("fail"));
    REQUIRE((std::is_same_v<decltype(reactor), Throw<int>>));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_WITH_AS(reactor.eval(), "fail", std::runtime_error);
  }
}

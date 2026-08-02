#include <exception>
#include <stdexcept>
#include <type_traits>
#include <doctest/doctest.h>
#include "Aspen/Reactor.hpp"
#include "Aspen/Throw.hpp"

using namespace Aspen;

TEST_SUITE("Throw") {
  TEST_CASE("evaluation") {
    static_assert(IsReactorOf<Throw<int>, int>);
    auto reactor = Throw<int>(std::runtime_error(""));
    static_assert(!is_noexcept_reactor_v<Throw<int>>);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("exception_ptr") {
    auto reactor = Throw<int>(
      std::make_exception_ptr(std::runtime_error("fail")));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_WITH_AS(reactor.eval(), "fail", std::runtime_error);
  }

  TEST_CASE("void_evaluation") {
    auto reactor = Throw<void>(std::logic_error("void"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::logic_error);
  }

  TEST_CASE("throws_function") {
    auto reactor = throws<int>(std::runtime_error("fail"));
    static_assert(std::is_same_v<decltype(reactor), Throw<int>>);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_WITH_AS(reactor.eval(), "fail", std::runtime_error);
  }

  TEST_CASE("null_exception_throws_a_runtime_error") {
    auto reactor = Throw<int>(std::exception_ptr());
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_WITH_AS(
      reactor.eval(), "Uninitialized.", std::runtime_error);
  }
}

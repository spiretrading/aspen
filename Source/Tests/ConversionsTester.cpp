#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"
#include "Aspen/Conversions.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Conversions") {
  TEST_CASE("converting_an_evaluation") {
    auto reactor = ConversionReactor(Constant(5), [] (int value) noexcept {
      return 2.5 * value;
    });
    REQUIRE((std::is_same_v<decltype(reactor)::Type, double>));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 12.5);
  }

  TEST_CASE("converting_every_evaluation") {
    auto queue = Shared(Queue<int>());
    auto reactor = ConversionReactor(queue, [] (int value) noexcept {
      return value + 1;
    });
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(1);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(reactor.eval() == 2);
    queue->push(9);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
    queue->set_complete();
    REQUIRE(reactor.commit(4) == State::COMPLETE);
    REQUIRE(reactor.eval() == 10);
  }

  TEST_CASE("an_evaluation_is_converted_once") {
    auto queue = Shared(Queue<int>());
    auto reactor = ConversionReactor(queue, [] (int value) noexcept {
      return value;
    });
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(&reactor.eval() == &reactor.eval());
  }

  TEST_CASE("a_conversion_returning_a_reference") {
    auto queue = Shared(Queue<std::string>());
    auto reactor = ConversionReactor(queue,
      [] (const std::string& value) noexcept -> const std::string& {
        return value;
      });
    REQUIRE((std::is_same_v<decltype(reactor)::Type, std::string>));
    queue->push("hello");
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == "hello");
    queue->push("world");
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == "world");
  }

  TEST_CASE("a_conversion_that_throws") {
    auto queue = Shared(Queue<int>());
    auto reactor = ConversionReactor(queue, [] (int value) {
      if(value < 0) {
        throw std::runtime_error("negative");
      }
      return value;
    });
    REQUIRE(!decltype(reactor)::is_noexcept);
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    queue->push(-1);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("a_child_that_throws") {
    auto queue = Shared(Queue<int>());
    auto reactor = ConversionReactor(queue, [] (int value) {
      return value;
    });
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("casting_to_another_type") {
    auto reactor = static_reactor_cast<double>(Constant(5));
    REQUIRE((std::is_same_v<decltype(reactor)::Type, double>));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5.0);
  }

  TEST_CASE("casting_to_the_same_type") {
    auto named = Constant(5);
    auto reactor = static_reactor_cast<int>(named);
    REQUIRE((std::is_same_v<decltype(reactor), Constant<int>>));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    auto temporary = static_reactor_cast<int>(Constant(9));
    REQUIRE((std::is_same_v<decltype(temporary), Constant<int>>));
    REQUIRE(temporary.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(temporary.eval() == 9);
  }
}

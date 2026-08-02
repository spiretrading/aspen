#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"
#include "Aspen/Conversions.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

namespace {
  struct Checked {
    int m_value;

    explicit Checked(int value)
        : m_value(value) {
      if(value < 0) {
        throw std::runtime_error("negative");
      }
    }
  };

  struct Required {
    int m_value;

    explicit Required(int value) noexcept
      : m_value(value) {}
  };
}

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
    queue->push(9);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 10);
    queue->set_complete();
    REQUIRE(reactor.commit(4) == State::COMPLETE);
  }

  TEST_CASE("propagating_a_continuation") {
    auto queue = Shared(Queue<int>());
    auto reactor = ConversionReactor(queue, [] (int value) noexcept {
      return value * 2;
    });
    queue->push(1);
    queue->push(2);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 4);
  }

  TEST_CASE("converting_once_per_evaluation") {
    auto queue = Shared(Queue<int>());
    auto conversions = std::make_shared<int>(0);
    auto reactor = ConversionReactor(queue, [conversions] (int value) noexcept {
      ++*conversions;
      return value;
    });
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(*conversions == 1);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(*conversions == 1);
    queue->push(2);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(*conversions == 2);
  }

  TEST_CASE("conversion_returning_a_reference") {
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

  TEST_CASE("conversion_exception") {
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

  TEST_CASE("child_exception") {
    auto queue = Shared(Queue<int>());
    auto reactor = ConversionReactor(queue, [] (int value) {
      return value;
    });
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("child_exception_with_a_noexcept_conversion") {
    auto queue = Shared(Queue<int>());
    auto reactor = ConversionReactor(queue, [] (int value) noexcept {
      return value;
    });
    REQUIRE(!decltype(reactor)::is_noexcept);
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

  TEST_CASE("converting_without_a_default") {
    auto reactor = ConversionReactor(Constant(5), [] (int value) noexcept {
      return Required(value);
    });
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval().m_value == 5);
  }

  TEST_CASE("casting_with_an_exception") {
    auto reactor = static_reactor_cast<Checked>(Constant(-1));
    REQUIRE(!decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("casting_a_value") {
    auto same = static_reactor_cast<int>(5);
    REQUIRE((std::is_same_v<decltype(same), Constant<int>>));
    REQUIRE(same.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(same.eval() == 5);
    auto converted = static_reactor_cast<double>(5);
    REQUIRE((std::is_same_v<decltype(converted)::Type, double>));
    REQUIRE(converted.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(converted.eval() == 5.0);
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

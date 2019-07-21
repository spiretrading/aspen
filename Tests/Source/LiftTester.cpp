#include <catch2/catch.hpp>
#include "Aspen/Constant.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Queue.hpp"

using namespace Aspen;

namespace {
  int no_parameter_function() {
    return 512;
  }

  int no_parameter_function_throw() {
    throw std::runtime_error("");
  }

  std::optional<int> no_parameter_function_empty() {
    return std::nullopt;
  }

  int square(int x) {
    return x * x;
  }
}

TEST_CASE("test_lift_no_parameters", "[Lift]") {
  auto reactor = Lift(no_parameter_function);
  REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 512);
}

TEST_CASE("test_lift_no_parameters_throw", "[Lift]") {
  auto reactor = Lift(no_parameter_function_throw);
  REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
}

TEST_CASE("test_lift_no_parameters_empty", "[Lift]") {
  auto reactor = Lift(no_parameter_function_empty);
  REQUIRE(reactor.commit(0) == State::COMPLETE_EMPTY);
}

TEST_CASE("test_lift_constant_argument", "[Lift]") {
  auto reactor = Lift(square, Constant(5));
  REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 25);
}

TEST_CASE("test_lift_one_argument_updates", "[Lift]") {
  auto queue = Queue<int>();
  auto reactor = Lift(square, &queue);
  auto trigger = Trigger();
  Trigger::set_trigger(trigger);
  REQUIRE(reactor.commit(0) == State::NONE);
  queue.push(10);
  REQUIRE(reactor.commit(1) == State::EVALUATED);
  REQUIRE(reactor.eval() == 100);
  REQUIRE(reactor.commit(2) == State::NONE);
  REQUIRE(reactor.eval() == 100);
  queue.push(5);
  REQUIRE(reactor.commit(3) == State::EVALUATED);
  REQUIRE(reactor.eval() == 25);
  REQUIRE(reactor.commit(4) == State::NONE);
  REQUIRE(reactor.eval() == 25);
  queue.set_complete(4);
  REQUIRE(reactor.commit(5) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == 16);
}

#include <doctest/doctest.h>

import <stdexcept>;
import Aspen;

using namespace Aspen;

namespace {
  int no_parameter_function() {
    return 512;
  }

  int no_parameter_function_throw() {
    throw std::runtime_error("");
  }

  int square(int x) {
    return x * x;
  }
}

TEST_SUITE("Lift") {
  TEST_CASE("lift_no_parameters") {
    auto reactor = Lift(no_parameter_function);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 512);
  }

  TEST_CASE("lift_no_parameters_throw") {
    auto reactor = Lift(no_parameter_function_throw);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("lift_constant_argument") {
    auto reactor = Lift(square, Constant(5));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 25);
  }

  TEST_CASE("lift_one_argument_updates") {
    auto queue = Shared(Queue<int>());
    auto reactor = Lift(square, queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 100);
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(reactor.eval() == 100);
    queue->push(5);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 25);
    REQUIRE(reactor.commit(4) == State::NONE);
    REQUIRE(reactor.eval() == 25);
    queue->set_complete(4);
    REQUIRE(reactor.commit(5) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 16);
  }

  TEST_CASE("complete_arguments") {
    auto queue = Shared(Queue<int>());
    queue->push(10);
    queue.commit(0);
    queue->set_complete();
    auto reactor = Lift(square, queue);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 100);
  }
}

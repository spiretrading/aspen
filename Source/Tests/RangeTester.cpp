#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Range") {
  TEST_CASE("backward_range") {
    auto reactor = range(constant(10), constant(9));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("empty_range") {
    auto reactor = range(constant(1), constant(1));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("single_range") {
    auto reactor = range(constant(1), constant(2));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
  }

  TEST_CASE("double_range") {
    auto reactor = range(constant(10), constant(12));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 11);
  }

  TEST_CASE("end_complete") {
    auto queue = Shared(Queue<int>());
    auto reactor = range(queue, constant(10));
    queue->push(8);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 8);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 9);
  }

  TEST_CASE("changing_start") {
    auto queue = Shared(Queue<int>());
    auto reactor = range(queue, constant(100));
    queue->push(8);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 8);
    queue->push(50);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 50);
    queue->push(20);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 51);
    queue->push(200);
    REQUIRE(reactor.commit(3) == State::COMPLETE);
    REQUIRE(reactor.eval() == 51);
  }

  TEST_CASE("changing_end") {
    auto start_queue = Shared(Queue<int>());
    auto end_queue = Shared(Queue<int>());
    auto reactor = range(start_queue, end_queue);
    start_queue->push(0);
    end_queue->push(10);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 0);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    start_queue->push(6);
    end_queue->push(9);
    REQUIRE(reactor.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 6);
    end_queue->push(3);
    REQUIRE(reactor.commit(3) == State::NONE);
    end_queue->push(7);
    REQUIRE(reactor.commit(4) == State::NONE);
    end_queue->push(8);
    REQUIRE(reactor.commit(5) == State::EVALUATED);
    REQUIRE(reactor.eval() == 7);
    end_queue->set_complete();
    REQUIRE(reactor.commit(6) == State::COMPLETE);
  }
}

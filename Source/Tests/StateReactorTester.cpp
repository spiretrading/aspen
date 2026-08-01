#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Last.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/StateReactor.hpp"

using namespace Aspen;

TEST_SUITE("StateReactor") {
  TEST_CASE("none") {
    auto reactor = StateReactor(none<int>());
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == State::COMPLETE);
  }

  TEST_CASE("constant") {
    auto reactor = StateReactor(constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == State::COMPLETE_EVALUATED);
  }

  TEST_CASE("value") {
    auto reactor = StateReactor(10);
    REQUIRE(is_noexcept_reactor_v<decltype(reactor)>);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == State::COMPLETE_EVALUATED);
  }

  TEST_CASE("exception") {
    auto queue = Shared(Queue<int>());
    auto reactor = StateReactor(queue);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == State::NONE);
    queue->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == State::COMPLETE_EVALUATED);
  }

  TEST_CASE("continuing_source") {
    auto reactor = StateReactor(last(chain(3, 1)));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == State::COMPLETE_EVALUATED);
  }

  TEST_CASE("series_of_states") {
    auto queue = Shared(Queue<int>());
    auto reactor = StateReactor(queue);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == State::NONE);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(reactor.commit(2) == State::NONE);
    queue->push(123);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == State::EVALUATED);
    queue->push(5);
    queue->push(10);
    REQUIRE(reactor.commit(4) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(5) == State::EVALUATED);
    REQUIRE(reactor.eval() == State::EVALUATED);
    queue->set_complete();
    REQUIRE(reactor.commit(6) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == State::COMPLETE);
  }
}

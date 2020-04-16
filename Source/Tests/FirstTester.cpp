#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"
#include "Aspen/First.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("First") {
  TEST_CASE("first_constant") {
    auto reactor = first(Constant(123));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("first_none") {
    auto reactor = first(None<int>());
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("first_multiple") {
    auto queue = Shared(Queue<int>());
    auto reactor = first(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(10);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
  }
}

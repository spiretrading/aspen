#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Sync.hpp"
#include "Aspen/Throw.hpp"
#include "Aspen/VectorSync.hpp"

using namespace Aspen;

TEST_SUITE("VectorSync") {
  TEST_CASE("empty_vector_sync") {
    auto list = std::vector<int>();
    auto reactors = std::vector<Box<int>>();
    reactors.push_back(box(constant(5)));
    reactors.push_back(box(none<int>()));
    reactors.push_back(box(constant(10)));
    auto reactor = VectorSync(list, std::move(reactors));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
    REQUIRE(reactor.eval() == std::vector{5, 0, 0});
    REQUIRE(list == std::vector{5, 0, 0});
  }

  TEST_CASE("single_vector_sync") {
    auto list = std::vector<int>();
    auto reactors = std::vector<Box<int>>();
    reactors.push_back(box(constant(5)));
    reactors.push_back(box(constant(12)));
    reactors.push_back(box(constant(3)));
    auto reactor = VectorSync(list, std::move(reactors));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == std::vector{5, 12, 3});
    REQUIRE(list == std::vector{5, 12, 3});
  }

  TEST_CASE("exception_vector_sync") {
    auto list = std::vector<int>();
    auto reactors = std::vector<Box<int>>();
    reactors.push_back(box(constant(5)));
    reactors.push_back(box(chain(throws<int>(std::runtime_error("fail")),
      constant(12))));
    reactors.push_back(box(constant(3)));
    auto reactor = VectorSync(list, std::move(reactors));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == std::vector{5, 12, 3});
    REQUIRE(list == std::vector{5, 12, 3});
  }

  TEST_CASE("persistent_exception_vector_sync") {
    auto queue = Shared(Queue<int>());
    auto list = std::vector<int>();
    auto reactors = std::vector<Box<int>>();
    reactors.push_back(box(queue));
    reactors.push_back(box(throws<int>(std::runtime_error("fail"))));
    auto reactor = VectorSync(list, std::move(reactors));
    queue->push(5);
    REQUIRE(has_evaluation(reactor.commit(0)));
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    queue->push(10);
    REQUIRE(has_evaluation(reactor.commit(1)));
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    REQUIRE(list[0] == 10);
  }

  TEST_CASE("recovering_exception_restores_evaluation") {
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    auto list = std::vector<int>();
    auto reactors = std::vector<Box<int>>();
    reactors.push_back(box(first));
    reactors.push_back(box(second));
    auto reactor = VectorSync(list, std::move(reactors));
    first->push(1);
    second->set_complete(std::runtime_error("fail"));
    REQUIRE(has_evaluation(reactor.commit(0)));
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    first->push(2);
    reactor.commit(1);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }
}

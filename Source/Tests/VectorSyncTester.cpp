#include <stdexcept>
#include <vector>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Cell.hpp"
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

  TEST_CASE("no_reactors_at_all") {
    auto list = std::vector<int>();
    auto reactor = VectorSync(list, std::vector<Box<int>>());
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval().empty());
    REQUIRE(list.empty());
  }

  TEST_CASE("a_vector_resized_to_the_reactors") {
    auto list = std::vector<int>{1, 2, 3, 4, 5};
    auto reactors = std::vector<Box<int>>();
    reactors.push_back(box(constant(7)));
    reactors.push_back(box(constant(8)));
    auto reactor = VectorSync(list, std::move(reactors));
    REQUIRE(list.size() == 2);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(list == std::vector{7, 8});
  }

  TEST_CASE("a_vector_of_a_different_element_type") {
    auto list = std::vector<double>();
    auto reactors = std::vector<Box<int>>();
    reactors.push_back(box(constant(5)));
    reactors.push_back(box(constant(12)));
    auto reactor = VectorSync(list, std::move(reactors));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(list == std::vector{5.0, 12.0});
  }

  TEST_CASE("reactors_that_cannot_throw") {
    auto list = std::vector<int>();
    auto first = Shared(Cell(1));
    auto second = Shared(Cell(2));
    auto reactors = std::vector<Shared<Cell<int>>>();
    reactors.push_back(first);
    reactors.push_back(second);
    auto reactor = VectorSync(list, std::move(reactors));
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(list == std::vector{1, 2});
    first->set(10);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(list == std::vector{10, 2});
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

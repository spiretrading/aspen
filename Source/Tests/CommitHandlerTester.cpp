#include <utility>
#include <vector>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/CommitHandler.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Last.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

TEST_SUITE("CommitHandler") {
  TEST_CASE("no_children") {
    auto reactor = CommitHandler<Box<void>>({});
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("immediate_completion") {
    auto queue = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue});
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("child_continuing_without_a_value") {
    auto reactor = CommitHandler(std::vector{last(chain(3, 1))});
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.get(0).eval() == 1);
  }

  TEST_CASE("completion") {
    auto queue = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue, queue});
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    queue->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("initial_evaluation") {
    auto queue_a = Shared(Queue<int>());
    auto queue_b = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue_a, queue_b});
    REQUIRE(reactor.commit(0) == State::NONE);
    queue_a->push(123);
    REQUIRE(reactor.commit(1) == State::NONE);
    queue_b->push(321);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
  }

  TEST_CASE("delayed_evaluation") {
    auto queue = Shared(Queue<int>());
    auto children = std::vector<Box<int>>();
    children.push_back(box(queue));
    children.push_back(box(constant(123)));
    auto reactor = CommitHandler(std::move(children));
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->push(123);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.commit(2) == State::NONE);
  }

  TEST_CASE("continuing_child") {
    auto queue = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue});
    queue->push(1);
    queue->push(2);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.commit(2) == State::NONE);
  }

  TEST_CASE("more_children_than_a_word") {
    auto queues = std::vector<Shared<Queue<int>>>();
    auto children = std::vector<Box<int>>();
    for(auto i = 0; i != 100; ++i) {
      queues.push_back(Shared(Queue<int>()));
      children.push_back(box(queues.back()));
    }
    auto reactor = CommitHandler(std::move(children));
    REQUIRE(reactor.size() == 100);
    REQUIRE(reactor.commit(0) == State::NONE);
    for(auto i = 0; i != 100; ++i) {
      queues[i]->push(i);
    }
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.get_evaluated().size() == 100);
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(reactor.get_evaluated().empty());
    queues[70]->push(7);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.get_evaluated().size() == 1);
    REQUIRE(reactor.get_evaluated().front() == 70);
    REQUIRE(reactor.get(70).eval() == 7);
  }

  TEST_CASE("move_construction") {
    auto queue = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue});
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    auto moved = std::move(reactor);
    REQUIRE(moved.commit(1) == State::NONE);
    queue->push(2);
    REQUIRE(moved.commit(2) == State::EVALUATED);
    REQUIRE(moved.get(0).eval() == 2);
  }

  TEST_CASE("move_assignment") {
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{first});
    first->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.get(0).eval() == 1);
    reactor = CommitHandler(std::vector{second});
    second->push(2);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.get(0).eval() == 2);
  }

  TEST_CASE("excluding_a_completed_child") {
    auto first = CountingReactor<int>(1, State::COMPLETE_EVALUATED);
    auto second = Shared(Queue<int>());
    auto children = std::vector<Box<int>>();
    children.push_back(box(first));
    children.push_back(box(second));
    auto reactor = CommitHandler(std::move(children));
    second->push(2);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(first.get_commits() == 1);
    second->push(3);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(first.get_commits() == 1);
    second->push(4);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(first.get_commits() == 1);
  }
}

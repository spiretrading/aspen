#include <utility>
#include <vector>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/CommitHandler.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("CommitHandler") {
  TEST_CASE("commit_empty_commit") {
    auto reactor = CommitHandler<Box<void>>({});
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("commit_immediate_complete") {
    auto queue = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue});
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("commit_complete") {
    auto queue = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue, queue});
    queue->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    queue->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("commit_empty_and_evaluated") {
    auto queue_a = Shared(Queue<int>());
    auto queue_b = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue_a, queue_b});
    REQUIRE(reactor.commit(0) == State::NONE);
    queue_a->push(123);
    REQUIRE(reactor.commit(1) == State::NONE);
    queue_b->push(321);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
  }

  TEST_CASE("commit_delayed_evaluation") {
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

  TEST_CASE("committing_a_child_that_continues") {
    auto queue = Shared(Queue<int>());
    auto reactor = CommitHandler(std::vector{queue});
    queue->push(1);
    queue->push(2);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.commit(2) == State::NONE);
  }

  TEST_CASE("committing_more_children_than_a_word_holds") {
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

  TEST_CASE("moving_a_handler_preserves_propagation") {
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
}

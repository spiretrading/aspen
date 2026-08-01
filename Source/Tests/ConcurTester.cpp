#include <cstdint>
#include <stdexcept>
#include <vector>
#include <doctest/doctest.h>
#include "Aspen/Aspen.hpp"
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

namespace {
  using Producer = Shared<Queue<SharedBox<int>>>;

  template<typename R>
  void absorb(R& reactor, std::uint64_t& sequence, std::size_t count) {
    for(auto i = std::size_t(0); i != count; ++i) {
      reactor.commit(sequence++);
    }
  }
}

TEST_SUITE("Concur") {
  TEST_CASE("no_children") {
    auto queue = Shared(Queue<SharedBox<int>>());
    auto reactor = concur(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("completion") {
    auto queue = Shared(Queue<SharedBox<int>>());
    auto reactor = concur(queue);
    queue->push(shared_box(constant(123)));
    queue->push(shared_box(none<int>()));
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("evaluating_in_turn") {
    auto producer = Producer();
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    producer->push(shared_box(first));
    producer->push(shared_box(second));
    producer->set_complete();
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 2);
    first->push(1);
    second->push(2);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(sequence++) == State::NONE);
  }

  TEST_CASE("child_waiting_its_turn") {
    auto producer = Producer();
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    producer->push(shared_box(first));
    producer->push(shared_box(second));
    producer->set_complete();
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 2);
    first->push(1);
    first->push(2);
    second->push(3);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("continuing_child") {
    auto producer = Producer();
    auto child = Shared(Queue<int>());
    producer->set_complete(shared_box(child));
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 1);
    child->push(1);
    child->push(2);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(sequence++) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(reactor.commit(sequence++) == State::NONE);
  }

  TEST_CASE("removing_a_completed_child") {
    auto producer = Producer();
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    producer->push(shared_box(first));
    producer->push(shared_box(second));
    producer->set_complete();
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 2);
    first->set_complete();
    REQUIRE(reactor.commit(sequence++) == State::NONE);
    second->push(7);
    REQUIRE(reactor.commit(sequence++) == State::EVALUATED);
    REQUIRE(reactor.eval() == 7);
  }

  TEST_CASE("destroying_a_completed_child") {
    auto producer = Producer();
    auto tracked = TrackedReactor<int>(9, State::COMPLETE_EVALUATED);
    auto token = tracked.get_token();
    auto second = Shared(Queue<int>());
    producer->push(shared_box(std::move(tracked)));
    producer->push(shared_box(second));
    producer->set_complete();
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 2);
    REQUIRE(!token.expired());
    second->push(3);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(token.expired());
  }

  TEST_CASE("completed_child_holding_an_evaluation") {
    auto producer = Producer();
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    producer->push(shared_box(first));
    producer->push(shared_box(second));
    producer->set_complete();
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 2);
    first->set_complete(9);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 9);
    REQUIRE(reactor.commit(sequence++) == State::NONE);
    second->push(3);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(reactor.commit(sequence++) == State::NONE);
    second->push(4);
    REQUIRE(reactor.commit(sequence++) == State::EVALUATED);
    REQUIRE(reactor.eval() == 4);
  }

  TEST_CASE("last_child_completing_with_a_value") {
    auto producer = Producer();
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    producer->push(shared_box(first));
    producer->push(shared_box(second));
    producer->set_complete();
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 2);
    first->set_complete(9);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 9);
    second->set_complete(3);
    REQUIRE(reactor.commit(sequence++) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
  }

  TEST_CASE("producer_exception") {
    auto producer = Producer();
    producer->set_complete(std::runtime_error("fail"));
    auto reactor = concur(producer);
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("child_exception") {
    auto producer = Producer();
    auto child = Shared(Queue<int>());
    producer->set_complete(shared_box(child));
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 1);
    child->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(sequence++) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("completed_producer") {
    auto producer = Producer();
    auto child = Shared(Queue<int>());
    producer->set_complete(shared_box(child));
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 1);
    child->push(1);
    REQUIRE(reactor.commit(sequence++) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    child->push(2);
    REQUIRE(reactor.commit(sequence++) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    child->set_complete();
    REQUIRE(reactor.commit(sequence++) == State::COMPLETE);
  }

  TEST_CASE("more_children_than_a_word") {
    auto producer = Producer();
    auto queues = std::vector<Shared<Queue<int>>>();
    for(auto i = 0; i != 100; ++i) {
      queues.push_back(Shared(Queue<int>()));
      producer->push(shared_box(queues.back()));
    }
    producer->set_complete();
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 105);
    queues[70]->push(7);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 7);
    queues[3]->push(3);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
  }

  TEST_CASE("reusing_a_slot") {
    auto producer = Producer();
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    auto third = Shared(Queue<int>());
    producer->push(shared_box(first));
    producer->push(shared_box(second));
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 2);
    first->set_complete();
    absorb(reactor, sequence, 3);
    producer->set_complete(shared_box(third));
    absorb(reactor, sequence, 2);
    third->push(3);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    second->push(2);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("child_produced_after_a_completion") {
    auto producer = Producer();
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    producer->push(shared_box(first));
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 1);
    first->set_complete(1);
    REQUIRE(reactor.commit(sequence++) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    producer->set_complete(shared_box(second));
    absorb(reactor, sequence, 2);
    second->push(2);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    second->set_complete();
    REQUIRE(reactor.commit(sequence++) == State::COMPLETE);
  }

  TEST_CASE("move_construction") {
    auto producer = Producer();
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    producer->push(shared_box(first));
    producer->push(shared_box(second));
    producer->set_complete();
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 2);
    first->push(1);
    second->push(2);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    auto moved = std::move(reactor);
    REQUIRE(moved.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(moved.eval() == 2);
  }

  TEST_CASE("child_produced_later") {
    auto producer = Producer();
    auto first = Shared(Queue<int>());
    auto second = Shared(Queue<int>());
    producer->push(shared_box(first));
    auto reactor = concur(producer);
    auto sequence = std::uint64_t(0);
    absorb(reactor, sequence, 1);
    first->push(1);
    REQUIRE(reactor.commit(sequence++) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    producer->set_complete(shared_box(second));
    absorb(reactor, sequence, 1);
    second->push(2);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("by_value_child") {
    auto reactor = Concur(Constant(ByValueReactor(CountedValue(1))));
    test_evaluation_lifetime(reactor);
  }

  TEST_CASE("child_completing_without_a_value") {
    auto queue = Shared(Queue<SharedBox<int>>());
    auto reactor = concur(queue);
    queue->push(shared_box(None<int>()));
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }
}

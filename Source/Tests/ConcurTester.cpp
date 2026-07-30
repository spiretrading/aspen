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

  /** Commits a reactor a number of times, discarding its state. */
  template<typename R>
  void absorb(R& reactor, std::uint64_t& sequence, std::size_t count) {
    for(auto i = std::size_t(0); i != count; ++i) {
      reactor.commit(sequence++);
    }
  }
}

TEST_SUITE("Concur") {
  TEST_CASE("empty_concur") {
    auto queue = Shared(Queue<SharedBox<int>>());
    auto reactor = concur(queue);
    REQUIRE(reactor.commit(0) == State::NONE);
    queue->set_complete();
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("loop_complete") {
    auto queue = Shared(Queue<SharedBox<int>>());
    auto reactor = concur(queue);
    queue->push(shared_box(constant(123)));
    queue->push(shared_box(none<int>()));
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 123);
    REQUIRE(reactor.commit(1) == State::COMPLETE);
    REQUIRE(reactor.eval() == 123);
  }

  TEST_CASE("children_are_evaluated_in_turn") {
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
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("a_child_waits_its_turn_while_another_has_values") {
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

  TEST_CASE("a_child_with_more_values_continues") {
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

  TEST_CASE("a_completed_child_is_removed") {
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

  TEST_CASE("an_evaluated_child_survives_its_completion") {
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
    REQUIRE(reactor.eval() == 9);
    second->push(3);
    REQUIRE(reactor.commit(sequence++) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(reactor.commit(sequence++) == State::NONE);
    second->push(4);
    REQUIRE(reactor.commit(sequence++) == State::EVALUATED);
    REQUIRE(reactor.eval() == 4);
  }

  TEST_CASE("a_producer_that_throws_is_ignored") {
    auto producer = Producer();
    producer->set_complete(std::runtime_error("fail"));
    auto reactor = concur(producer);
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("a_child_that_throws_is_evaluated") {
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

  TEST_CASE("children_outlive_a_completed_producer") {
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

  TEST_CASE("evaluating_more_children_than_a_word_holds") {
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

  TEST_CASE("a_slot_is_reused_by_a_later_child") {
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

  TEST_CASE("a_child_produced_after_another_completes") {
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

  TEST_CASE("moving_a_concur_resumes_where_it_left_off") {
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

  TEST_CASE("a_child_produced_later_is_evaluated") {
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
  TEST_CASE("concur_children_evaluating_by_value") {
    auto reactor = Concur(Constant(ByValueReactor(CountedValue(1))));
    test_evaluation_lifetime(reactor);
  }

  TEST_CASE("a_child_completing_without_evaluating") {
    auto queue = Shared(Queue<SharedBox<int>>());
    auto reactor = concur(queue);
    queue->push(shared_box(None<int>()));
    queue->set_complete();
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

}

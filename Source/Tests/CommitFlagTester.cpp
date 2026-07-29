#include <atomic>
#include <cstdint>
#include <memory>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/StaticCommitHandler.hpp"

using namespace Aspen;

namespace {

  /** A reactor that counts how many times it has been committed. */
  template<typename T>
  class Counter {
    public:
      using Type = T;

      explicit Counter(Shared<Cell<T>> cell)
        : m_cell(std::move(cell)),
          m_count(std::make_shared<int>(0)) {}

      const std::shared_ptr<int>& get_count() const {
        return m_count;
      }

      State commit(std::uint64_t sequence) noexcept {
        ++*m_count;
        return m_cell.commit(sequence);
      }

      eval_result_t<Type> eval() const noexcept {
        return m_cell.eval();
      }

    private:
      Shared<Cell<T>> m_cell;
      std::shared_ptr<int> m_count;
  };

  auto add = [] (int left, int right) noexcept {
    return left + right;
  };
}

TEST_SUITE("CommitFlag") {
  TEST_CASE("unraised_children_are_skipped") {
    auto left = Shared(Cell(1));
    auto right = Shared(Cell(2));
    auto left_counter = Counter(left);
    auto right_counter = Counter(right);
    auto left_count = left_counter.get_count();
    auto right_count = right_counter.get_count();
    auto reactor = Lift(add, std::move(left_counter), std::move(right_counter));
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 3);
    REQUIRE(*left_count == 1);
    REQUIRE(*right_count == 1);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(*left_count == 1);
    REQUIRE(*right_count == 1);
    left->set(10);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 12);
    REQUIRE(*left_count == 2);
    REQUIRE(*right_count == 1);
    right->set(20);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 30);
    REQUIRE(*left_count == 2);
    REQUIRE(*right_count == 2);
  }

  TEST_CASE("updates_propagate_through_nested_reactors") {
    auto cell = Shared(Cell(1));
    auto counter = Counter(cell);
    auto count = counter.get_count();
    auto reactor =
      Lift(add, Lift(add, std::move(counter), constant(0)), constant(100));
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 101);
    REQUIRE(*count == 1);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(*count == 1);
    cell->set(5);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 105);
    REQUIRE(*count == 2);
  }

  TEST_CASE("shared_children_evaluate_for_every_dependent") {
    auto cell = Shared(Cell(1));
    auto counter = Shared(Counter(cell));
    auto count = counter->get_count();
    auto reactor = Lift(add, counter, counter);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
    REQUIRE(*count == 1);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(*count == 1);
    cell->set(7);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 14);
    REQUIRE(*count == 2);
    REQUIRE(reactor.commit(3) == State::NONE);
    REQUIRE(*count == 2);
  }

  TEST_CASE("diamond_dependencies_remain_glitch_free") {
    auto cell = Shared(Cell(1));
    auto doubled = Shared(Lift([] (int value) noexcept {
      return 2 * value;
    }, cell));
    auto tripled = Shared(Lift([] (int value) noexcept {
      return 3 * value;
    }, cell));
    auto reactor = Lift(add, doubled, tripled);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    cell->set(10);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 50);
    REQUIRE(reactor.commit(2) == State::NONE);
    cell->set(100);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 500);
  }

  TEST_CASE("moving_a_committed_reactor_preserves_propagation") {
    auto cell = Shared(Cell(1));
    auto reactor = Lift(add, Lift(add, cell, constant(0)), constant(100));
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 101);
    auto moved = std::move(reactor);
    cell->set(5);
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved.eval() == 105);
    REQUIRE(moved.commit(2) == State::NONE);
    cell->set(7);
    REQUIRE(moved.commit(3) == State::EVALUATED);
    REQUIRE(moved.eval() == 107);
  }

  TEST_CASE("copying_a_committed_reactor_preserves_propagation") {
    auto cell = Shared(Cell(1));
    auto reactor = Lift(add, Lift(add, cell, constant(0)), constant(100));
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    auto copy = reactor;
    cell->set(5);
    REQUIRE(copy.commit(1) == State::EVALUATED);
    REQUIRE(copy.eval() == 105);
    REQUIRE(copy.commit(2) == State::NONE);
    cell->set(7);
    REQUIRE(copy.commit(3) == State::EVALUATED);
    REQUIRE(copy.eval() == 107);
  }

  TEST_CASE("raising_propagates_to_every_dependent") {
    auto root = CommitFlag();
    auto a = CommitFlag();
    auto b = CommitFlag();
    auto hub = CommitFlag();
    a.set_parent(&root);
    b.set_parent(&root);
    hub.add_parent(a);
    hub.add_parent(b);
    root.clear();
    a.clear();
    b.clear();
    hub.clear();
    hub.raise();
    REQUIRE(hub.is_raised());
    REQUIRE(a.is_raised());
    REQUIRE(b.is_raised());
    REQUIRE(root.is_raised());
    a.clear();
    b.clear();
    hub.clear();
    hub.raise();
    REQUIRE(a.is_raised());
    REQUIRE(b.is_raised());
  }

  TEST_CASE("dependents_can_be_removed") {
    auto root = CommitFlag();
    auto a = CommitFlag();
    auto b = CommitFlag();
    auto c = CommitFlag();
    auto hub = CommitFlag();
    a.set_parent(&root);
    b.set_parent(&root);
    c.set_parent(&root);
    hub.add_parent(a);
    hub.add_parent(b);
    hub.add_parent(c);
    auto reset = [&] {
      root.clear();
      a.clear();
      b.clear();
      c.clear();
      hub.clear();
    };
    reset();
    hub.raise();
    REQUIRE(a.is_raised());
    REQUIRE(b.is_raised());
    REQUIRE(c.is_raised());
    hub.remove_parent(b);
    reset();
    hub.raise();
    REQUIRE(a.is_raised());
    REQUIRE(!b.is_raised());
    REQUIRE(c.is_raised());
    hub.remove_parent(a);
    reset();
    hub.raise();
    REQUIRE(!a.is_raised());
    REQUIRE(c.is_raised());
  }

  TEST_CASE("a_slot_records_a_raised_flag") {
    auto word = std::atomic_uint64_t(0);
    auto flag = CommitFlag();
    flag.clear();
    flag.set_slot(&word, 3);
    REQUIRE(word.load() == 0);
    flag.raise();
    REQUIRE(word.load() == std::uint64_t(1) << 3);
    flag.clear();
    flag.raise();
    REQUIRE(word.load() == std::uint64_t(1) << 3);
  }

  TEST_CASE("a_slot_records_a_flag_that_is_already_raised") {
    auto word = std::atomic_uint64_t(0);
    auto flag = CommitFlag();
    flag.set_slot(&word, 5);
    REQUIRE(word.load() == std::uint64_t(1) << 5);
  }

  TEST_CASE("adding_a_dependent_reports_a_raised_flag") {
    auto raised_parent = CommitFlag();
    auto hub = CommitFlag();
    raised_parent.clear();
    hub.add_parent(raised_parent);
    REQUIRE(raised_parent.is_raised());
    auto cleared_parent = CommitFlag();
    auto cleared_hub = CommitFlag();
    cleared_parent.clear();
    cleared_hub.clear();
    cleared_hub.add_parent(cleared_parent);
    REQUIRE(!cleared_parent.is_raised());
  }

  TEST_CASE("raising_stops_at_raised_ancestors") {
    auto root = CommitFlag();
    auto branch = CommitFlag();
    auto leaf = CommitFlag();
    branch.set_parent(&root);
    leaf.set_parent(&branch);
    root.clear();
    branch.clear();
    leaf.clear();
    leaf.raise();
    REQUIRE(root.is_raised());
    root.clear();
    leaf.raise();
    REQUIRE(!root.is_raised());
    leaf.clear();
    leaf.raise();
    REQUIRE(!root.is_raised());
    branch.clear();
    leaf.clear();
    leaf.raise();
    REQUIRE(root.is_raised());
  }
}

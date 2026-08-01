#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Branch.hpp"
#include "Aspen/Cell.hpp"
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

namespace {
  class Counter {
    public:
      using Type = int;

      explicit Counter(State state)
        : m_state(state),
          m_count(std::make_shared<int>(0)) {}

      const std::shared_ptr<int>& get_count() const {
        return m_count;
      }

      State commit(std::uint64_t sequence) noexcept {
        ++*m_count;
        return m_state;
      }

      const int& eval() const noexcept {
        return *m_count;
      }

    private:
      State m_state;
      std::shared_ptr<int> m_count;
  };
}

TEST_SUITE("Branch") {
  TEST_CASE("unraised_branch") {
    auto counter = Counter(State::NONE);
    auto count = counter.get_count();
    auto branch = Branch(std::move(counter));
    REQUIRE(branch.commit(0) == State::NONE);
    REQUIRE(*count == 1);
    REQUIRE(branch.commit(1) == State::NONE);
    REQUIRE(*count == 1);
    REQUIRE(branch.commit(2) == State::NONE);
    REQUIRE(*count == 1);
  }

  TEST_CASE("raising") {
    auto cell = Shared(Cell(1));
    auto branch = Branch(cell);
    REQUIRE(branch.commit(0) == State::EVALUATED);
    REQUIRE(branch->eval() == 1);
    REQUIRE(branch.commit(1) == State::NONE);
    cell->set(2);
    REQUIRE(branch.commit(2) == State::EVALUATED);
    REQUIRE(branch->eval() == 2);
    REQUIRE(branch.commit(3) == State::NONE);
  }

  TEST_CASE("completion") {
    auto branch = Branch(constant(5));
    REQUIRE(branch.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(branch->eval() == 5);
    REQUIRE(branch.commit(1) == State::COMPLETE);
    REQUIRE(branch.commit(2) == State::COMPLETE);
  }

  TEST_CASE("continuation") {
    auto counter = Counter(State::CONTINUE_EVALUATED);
    auto count = counter.get_count();
    auto branch = Branch(std::move(counter));
    REQUIRE(branch.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(*count == 1);
    REQUIRE(branch.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(*count == 2);
    REQUIRE(branch.commit(2) == State::CONTINUE_EVALUATED);
    REQUIRE(*count == 3);
  }

  TEST_CASE("move_construction") {
    auto cell = Shared(Cell(1));
    auto branch = Branch(cell);
    REQUIRE(branch.commit(0) == State::EVALUATED);
    auto moved = std::move(branch);
    cell->set(2);
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved->eval() == 2);
    REQUIRE(moved.commit(2) == State::NONE);
    cell->set(3);
    REQUIRE(moved.commit(3) == State::EVALUATED);
    REQUIRE(moved->eval() == 3);
  }

  TEST_CASE("destroying_a_moved_from_branch") {
    auto cell = Shared(Cell(1));
    auto branch = std::optional(Branch(cell));
    REQUIRE(branch->commit(0) == State::EVALUATED);
    auto moved = std::move(*branch);
    branch.reset();
    cell->set(2);
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved->eval() == 2);
  }

  TEST_CASE("completed_branch_with_a_slot") {
    auto word = std::atomic_uint64_t(0);
    auto counter = Counter(State::COMPLETE_EVALUATED);
    auto count = counter.get_count();
    auto branch = Branch(std::move(counter));
    branch.set_slot(&word, 0);
    REQUIRE(branch.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(*count == 1);
    REQUIRE(branch.commit(1) == State::COMPLETE);
    REQUIRE(*count == 1);
  }

  TEST_CASE("assignment") {
    auto first = Shared(Cell(1));
    auto second = Shared(Cell(2));
    auto third = Shared(Cell(3));
    auto branch = Branch(first);
    REQUIRE(branch.commit(0) == State::EVALUATED);
    REQUIRE(branch->eval() == 1);
    REQUIRE(branch.commit(1) == State::NONE);
    branch = Branch(second);
    REQUIRE(branch.commit(2) == State::EVALUATED);
    REQUIRE(branch->eval() == 2);
    REQUIRE(branch.commit(3) == State::NONE);
    auto other = Branch(third);
    branch = other;
    REQUIRE(branch.commit(4) == State::EVALUATED);
    REQUIRE(branch->eval() == 3);
    REQUIRE(branch.commit(5) == State::NONE);
  }

  TEST_CASE("reporting_to_a_flag") {
    auto cell = Shared(Cell(1));
    auto branch = Branch(cell);
    auto first = CommitFlag();
    {
      auto scope = CommitFlagScope(first);
      REQUIRE(branch.commit(0) == State::EVALUATED);
    }
    cell->set(2);
    auto second = CommitFlag();
    {
      auto scope = CommitFlagScope(second);
      REQUIRE(branch.commit(1) == State::EVALUATED);
    }
    second.clear();
    cell->set(3);
    REQUIRE(second.is_raised());
  }

  TEST_CASE("copy_construction") {
    auto cell = Shared(Cell(1));
    auto branch = Branch(cell);
    REQUIRE(branch.commit(0) == State::EVALUATED);
    auto copy = branch;
    cell->set(2);
    REQUIRE(copy.commit(1) == State::EVALUATED);
    REQUIRE(copy->eval() == 2);
    REQUIRE(copy.commit(2) == State::NONE);
    cell->set(3);
    REQUIRE(copy.commit(3) == State::EVALUATED);
    REQUIRE(copy->eval() == 3);
  }
}

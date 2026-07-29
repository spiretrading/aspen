#include <cstdint>
#include <memory>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Branch.hpp"
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

namespace {

  /** A reactor that counts how many times it has been committed. */
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
  TEST_CASE("an_unraised_branch_is_skipped") {
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

  TEST_CASE("raising_a_branch_commits_it_again") {
    auto cell = Shared(Cell(1));
    auto branch = Branch(cell);
    REQUIRE(branch.commit(0) == State::EVALUATED);
    REQUIRE(branch->eval() == 1);
    REQUIRE(branch.commit(1) == State::NONE);
    cell->set(2);
    REQUIRE(branch.commit(2) == State::EVALUATED);
    REQUIRE(branch->eval() == 2);
    REQUIRE(branch.commit(3) == State::NONE);
    REQUIRE(branch->eval() == 2);
  }

  TEST_CASE("completion_survives_a_skipped_commit") {
    auto branch = Branch(constant(5));
    REQUIRE(branch.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(branch.commit(1) == State::COMPLETE);
    REQUIRE(branch.commit(2) == State::COMPLETE);
    REQUIRE(branch->eval() == 5);
  }

  TEST_CASE("a_continuation_is_committed_again") {
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

  TEST_CASE("moving_a_branch_preserves_propagation") {
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

  TEST_CASE("assigning_a_branch_commits_it_again") {
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

  TEST_CASE("copying_a_branch_preserves_propagation") {
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

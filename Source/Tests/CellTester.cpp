#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/CommitFlag.hpp"

using namespace Aspen;
using namespace std::string_literals;

namespace {
  struct Checked {
    int m_value;

    explicit Checked(int value)
        : m_value(value) {
      if(value < 0) {
        throw std::runtime_error("negative");
      }
    }
  };
}

TEST_SUITE("Cell") {
  TEST_CASE("immediate_completion") {
    auto cell = Cell<int>();
    cell.set_complete();
    REQUIRE(cell.commit(0) == State::COMPLETE);
  }

  TEST_CASE("value") {
    auto cell = Cell(123);
    cell.set_complete();
    REQUIRE(cell.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(cell.eval() == 123);
  }

  TEST_CASE("value_then_completion") {
    auto cell = Cell(321);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    REQUIRE(cell.eval() == 321);
    cell.set_complete();
    REQUIRE(cell.commit(1) == State::COMPLETE);
  }

  TEST_CASE("completion_while_empty") {
    auto cell = Cell<int>();
    REQUIRE(cell.commit(0) == State::NONE);
    cell.set_complete();
    REQUIRE(cell.commit(1) == State::COMPLETE);
  }

  TEST_CASE("value_while_empty") {
    auto cell = Cell<int>();
    REQUIRE(cell.commit(0) == State::NONE);
    cell.set(1);
    REQUIRE(cell.commit(1) == State::EVALUATED);
    REQUIRE(cell.eval() == 1);
  }

  TEST_CASE("completion_with_a_value_while_empty") {
    auto cell = Cell<int>();
    REQUIRE(cell.commit(0) == State::NONE);
    cell.set_complete(1);
    REQUIRE(cell.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(cell.eval() == 1);
  }

  TEST_CASE("setting_twice_before_a_commit") {
    auto cell = Cell<int>();
    REQUIRE(cell.commit(0) == State::NONE);
    cell.set(1);
    cell.set(2);
    cell.set(3);
    REQUIRE(cell.commit(1) == State::EVALUATED);
    REQUIRE(cell.eval() == 3);
    REQUIRE(cell.commit(2) == State::NONE);
  }

  TEST_CASE("setting_before_the_first_commit") {
    auto cell = Cell<int>();
    cell.set(1);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    REQUIRE(cell.eval() == 1);
  }

  TEST_CASE("in_place_construction") {
    auto cell = Cell<std::string>(std::in_place, 3, 'a');
    REQUIRE(cell.commit(0) == State::EVALUATED);
    REQUIRE(cell.eval() == "aaa"s);
    cell.emplace(2, 'b');
    REQUIRE(cell.commit(1) == State::EVALUATED);
    REQUIRE(cell.eval() == "bb"s);
    cell.emplace_complete(4, 'c');
    REQUIRE(cell.commit(2) == State::COMPLETE_EVALUATED);
    REQUIRE(cell.eval() == "cccc"s);
  }

  TEST_CASE("copy_construction") {
    auto cell = Cell(1);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    cell.set(2);
    auto copy = cell;
    REQUIRE(copy.commit(0) == State::EVALUATED);
    REQUIRE(copy.eval() == 2);
    REQUIRE(copy.commit(1) == State::NONE);
    REQUIRE(cell.commit(1) == State::EVALUATED);
    REQUIRE(cell.eval() == 2);
  }

  TEST_CASE("copy_construction_after_a_commit") {
    auto cell = Cell(1);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    REQUIRE(cell.eval() == 1);
    auto copy = cell;
    REQUIRE(copy.commit(0) == State::EVALUATED);
    REQUIRE(copy.eval() == 1);
  }

  TEST_CASE("move_construction_after_a_commit") {
    auto cell = Cell(1);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    REQUIRE(cell.eval() == 1);
    auto moved = Cell(std::move(cell));
    REQUIRE(moved.commit(0) == State::EVALUATED);
    REQUIRE(moved.eval() == 1);
    auto completed = Cell<int>();
    completed.set_complete(2);
    REQUIRE(completed.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(completed.eval() == 2);
    auto moved_completion = Cell<int>(std::move(completed));
    REQUIRE(moved_completion.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(moved_completion.eval() == 2);
  }

  TEST_CASE("assignment") {
    auto cell = Cell(1);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    REQUIRE(cell.eval() == 1);
    auto other = Cell(2);
    cell = other;
    REQUIRE(cell.commit(1) == State::EVALUATED);
    REQUIRE(cell.eval() == 2);
    cell = Cell(3);
    REQUIRE(cell.commit(2) == State::EVALUATED);
    REQUIRE(cell.eval() == 3);
    cell = cell;
    REQUIRE(cell.commit(3) == State::NONE);
  }

  TEST_CASE("raising_on_an_update") {
    auto flag = CommitFlag();
    auto cell = Cell(1);
    {
      auto scope = CommitFlagScope(flag);
      REQUIRE(cell.commit(0) == State::EVALUATED);
    }
    flag.clear();
    cell.set(2);
    REQUIRE(flag.is_raised());
    flag.clear();
    auto other = Cell(3);
    cell = other;
    REQUIRE(flag.is_raised());
    flag.clear();
    cell = Cell(4);
    REQUIRE(flag.is_raised());
    flag.clear();
    cell.emplace(5);
    REQUIRE(flag.is_raised());
    flag.clear();
    cell.set_complete();
    REQUIRE(flag.is_raised());
  }

  TEST_CASE("reporting_to_a_flag") {
    auto first = CommitFlag();
    auto second = CommitFlag();
    auto cell = Cell(1);
    {
      auto scope = CommitFlagScope(first);
      REQUIRE(cell.commit(0) == State::EVALUATED);
    }
    {
      auto scope = CommitFlagScope(second);
      REQUIRE(cell.commit(1) == State::NONE);
    }
    first.clear();
    second.clear();
    cell.set(2);
    REQUIRE(!first.is_raised());
    REQUIRE(second.is_raised());
  }

  TEST_CASE("setting_after_completion") {
    auto flag = CommitFlag();
    auto cell = Cell<int>();
    {
      auto scope = CommitFlagScope(flag);
      cell.set_complete(1);
      REQUIRE(cell.commit(0) == State::COMPLETE_EVALUATED);
      REQUIRE(cell.eval() == 1);
    }
    flag.clear();
    cell.set(2);
    REQUIRE(!flag.is_raised());
  }

  TEST_CASE("completing_with_a_throwing_value") {
    auto flag = CommitFlag();
    auto cell = Cell<Checked>();
    {
      auto scope = CommitFlagScope(flag);
      REQUIRE(cell.commit(0) == State::NONE);
    }
    flag.clear();
    REQUIRE_THROWS_AS(cell.emplace_complete(-1), std::runtime_error);
    REQUIRE(!flag.is_raised());
    cell.emplace(2);
    REQUIRE(flag.is_raised());
    REQUIRE(cell.commit(1) == State::EVALUATED);
    REQUIRE(cell.eval().m_value == 2);
  }

  TEST_CASE("assignment_before_a_commit") {
    auto destination = Cell(1);
    REQUIRE(destination.commit(0) == State::EVALUATED);
    REQUIRE(destination.eval() == 1);
    auto source = Cell(2);
    destination = source;
    REQUIRE(destination.eval() == 1);
    REQUIRE(destination.commit(1) == State::EVALUATED);
    REQUIRE(destination.eval() == 2);
  }

}

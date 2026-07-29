#include <exception>
#include <string>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/CommitFlag.hpp"

using namespace Aspen;
using namespace std::string_literals;

TEST_SUITE("Cell") {
  TEST_CASE("cell_immediate_complete") {
    auto cell = Cell<int>();
    cell.set_complete();
    REQUIRE(cell.commit(0) == State::COMPLETE);
  }

  TEST_CASE("cell_single_value") {
    auto cell = Cell(123);
    cell.set_complete();
    REQUIRE(cell.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(cell.eval() == 123);
  }

  TEST_CASE("cell_single_value_then_complete") {
    auto cell = Cell(321);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    REQUIRE(cell.eval() == 321);
    cell.set_complete();
    REQUIRE(cell.commit(1) == State::COMPLETE);
    REQUIRE(cell.eval() == 321);
  }

  TEST_CASE("cell_empty_then_complete") {
    auto cell = Cell<int>();
    REQUIRE(cell.commit(0) == State::NONE);
    cell.set_complete();
    REQUIRE(cell.commit(1) == State::COMPLETE);
  }

  TEST_CASE("cell_empty_then_evaluated") {
    auto cell = Cell<int>();
    REQUIRE(cell.commit(0) == State::NONE);
    cell.set(1);
    REQUIRE(cell.commit(1) == State::EVALUATED);
    REQUIRE(cell.eval() == 1);
  }

  TEST_CASE("cell_empty_then_complete_evaluated") {
    auto cell = Cell<int>();
    REQUIRE(cell.commit(0) == State::NONE);
    cell.set_complete(1);
    REQUIRE(cell.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(cell.eval() == 1);
  }

  TEST_CASE("setting_a_cell_more_than_once_before_a_commit") {
    auto cell = Cell<int>();
    REQUIRE(cell.commit(0) == State::NONE);
    cell.set(1);
    cell.set(2);
    cell.set(3);
    REQUIRE(cell.commit(1) == State::EVALUATED);
    REQUIRE(cell.eval() == 3);
    REQUIRE(cell.commit(2) == State::NONE);
    REQUIRE(cell.eval() == 3);
  }

  TEST_CASE("setting_a_cell_before_it_is_committed") {
    auto cell = Cell<int>();
    cell.set(1);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    REQUIRE(cell.eval() == 1);
  }

  TEST_CASE("constructing_a_value_in_place") {
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

  TEST_CASE("copying_a_cell") {
    auto cell = Cell(1);
    REQUIRE(cell.commit(0) == State::EVALUATED);
    cell.set(2);
    auto copy = cell;
    REQUIRE(copy.commit(0) == State::EVALUATED);
    REQUIRE(copy.eval() == 2);
    REQUIRE(copy.commit(1) == State::NONE);
    REQUIRE(cell.commit(1) == State::EVALUATED);
    REQUIRE(cell.eval() == 2);
    auto moved = std::move(copy);
    REQUIRE(moved.eval() == 2);
  }

  TEST_CASE("assigning_a_cell") {
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
    REQUIRE(cell.eval() == 3);
  }

  TEST_CASE("updating_a_cell_requires_a_commit") {
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

  TEST_CASE("a_cell_reports_to_its_most_recent_commit") {
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
}

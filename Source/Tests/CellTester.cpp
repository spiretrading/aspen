#include <exception>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"

using namespace Aspen;

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
}

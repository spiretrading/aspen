#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Discard.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Discard") {
  TEST_CASE("flipping_discard") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    series->push(1);
    series->push(2);
    series->push(3);
    series->push(4);
    auto reactor = discard(toggle, series);
    REQUIRE(reactor.commit(0) == State::CONTINUE);
    toggle->push(false);
    REQUIRE(reactor.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 2);
    toggle->push(true);
    REQUIRE(reactor.commit(2) == State::CONTINUE);
    REQUIRE(reactor.eval() == 2);
    toggle->push(false);
    REQUIRE(reactor.commit(3) == State::EVALUATED);
    REQUIRE(reactor.eval() == 4);
  }

  TEST_CASE("discard_noexcept_sources") {
    auto toggle = Shared(Cell(true));
    auto series = Shared(Cell(5));
    auto reactor = discard(toggle, series);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::NONE);
    toggle->set(false);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    series->set(6);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 6);
    toggle->set(true);
    REQUIRE(reactor.commit(3) == State::NONE);
    REQUIRE(reactor.eval() == 6);
  }

  TEST_CASE("discard_propagates_a_toggle_exception") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    series->push(1);
    series->set_complete();
    toggle->set_complete(std::runtime_error("fail"));
    auto reactor = discard(toggle, series);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("discard_propagates_a_series_exception") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    toggle->push(false);
    toggle->set_complete();
    series->set_complete(std::runtime_error("fail"));
    auto reactor = discard(toggle, series);
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("discard_drops_an_exception_while_toggled") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    toggle->push(false);
    series->push(1);
    auto reactor = discard(toggle, series);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    toggle->push(true);
    series->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(reactor.eval() == 1);
  }
}

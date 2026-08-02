#include <utility>
#include <stdexcept>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Perpetual.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Switch.hpp"
#include "Aspen/Tests/ReactorTests.hpp"

using namespace Aspen;
using namespace Aspen::Tests;

namespace {
  class Series {
    public:
      using Type = int;

      Series()
        : m_violations(std::make_shared<int>(0)),
          m_state(State::NONE),
          m_value(0) {}

      const std::shared_ptr<int>& get_violations() const {
        return m_violations;
      }

      void push(int value) {
        m_next = value;
      }

      State commit(std::uint64_t sequence) noexcept {
        if(m_next) {
          m_value = *m_next;
          m_next = std::nullopt;
          m_state = State::CONTINUE_EVALUATED;
        } else {
          m_state = State::NONE;
        }
        return m_state;
      }

      const int& eval() const noexcept {
        if(!has_evaluation(m_state)) {
          ++*m_violations;
        }
        return m_value;
      }

    private:
      std::shared_ptr<int> m_violations;
      std::optional<int> m_next;
      State m_state;
      int m_value;
  };
}

TEST_SUITE("Switch") {
  TEST_CASE("no_values") {
    auto reactor = Switch(Constant(false), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("toggle_that_is_on") {
    auto reactor = Switch(Constant(true), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
  }

  TEST_CASE("empty_series") {
    auto reactor = Switch(none<bool>(), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("toggling") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = Switch(toggle, series);
    REQUIRE(reactor.commit(0) == State::NONE);
    toggle->push(true);
    REQUIRE(reactor.commit(1) == State::NONE);
    series->push(321);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 321);
    toggle->push(false);
    REQUIRE(reactor.commit(3) == State::NONE);
    toggle->push(true);
    REQUIRE(reactor.commit(4) == State::EVALUATED);
    REQUIRE(reactor.eval() == 321);
  }

  TEST_CASE("toggling_on_without_a_series_evaluation") {
    auto toggle = Shared(Queue<bool>());
    auto series = Series();
    auto violations = series.get_violations();
    series.push(321);
    auto reactor = Switch(toggle, std::move(series));
    toggle->push(true);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 321);
    toggle->push(false);
    REQUIRE(reactor.commit(1) == State::NONE);
    toggle->push(true);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 321);
    REQUIRE(*violations == 0);
  }

  TEST_CASE("toggle_completing_while_on") {
    auto series = Shared(Queue<int>());
    auto reactor = Switch(Constant(true), series);
    REQUIRE(reactor.commit(0) == State::NONE);
    series->push(5);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    series->push(6);
    REQUIRE(reactor.commit(2) == State::EVALUATED);
    REQUIRE(reactor.eval() == 6);
    series->set_complete();
    REQUIRE(reactor.commit(3) == State::COMPLETE);
  }

  TEST_CASE("toggle_exception") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = Switch(toggle, series);
    series->push(1);
    toggle->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
  }

  TEST_CASE("toggle_exception_after_a_value") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    auto reactor = Switch(toggle, series);
    toggle->push(true);
    series->push(1);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 1);
    toggle->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("series_exception") {
    auto series = Shared(Queue<int>());
    auto reactor = Switch(Constant(true), series);
    REQUIRE(reactor.commit(0) == State::NONE);
    series->set_complete(std::runtime_error("fail"));
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  }

  TEST_CASE("noexcept_children") {
    auto toggle = Shared(Cell(true));
    auto series = Shared(Cell(5));
    auto reactor = Switch(toggle, series);
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    series->set(6);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 6);
    toggle->set(false);
    REQUIRE(reactor.commit(2) == State::NONE);
  }

  TEST_CASE("switch_function") {
    auto reactor = switch_(Constant(true), Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
  }

  TEST_CASE("noexcept_series") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Cell(5));
    auto reactor = switch_(toggle, series);
    REQUIRE(decltype(reactor)::is_noexcept);
    toggle->push(true);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
  }

  TEST_CASE("toggle_exception_with_a_held_value") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Cell(5));
    auto reactor = switch_(toggle, series);
    toggle->push(true);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    toggle->set_complete(std::runtime_error("bad"));
    REQUIRE(reactor.commit(1) == State::COMPLETE);
  }

  TEST_CASE("series_gated_by_the_toggle") {
    auto toggle = Shared(Queue<bool>());
    auto series = CountingReactor<int>(1, State::CONTINUE_EVALUATED);
    auto reactor = Switch(toggle, series);
    toggle->push(true);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(series.get_commits() == 1);
    toggle->push(false);
    REQUIRE(reactor.commit(1) == State::NONE);
    REQUIRE(series.get_commits() == 1);
    REQUIRE(reactor.commit(2) == State::NONE);
    REQUIRE(series.get_commits() == 1);
    toggle->push(true);
    REQUIRE(has_evaluation(reactor.commit(3)));
    REQUIRE(series.get_commits() == 2);
  }

  TEST_CASE("continuing_toggle") {
    auto toggle = Shared(Queue<bool>());
    auto series = Shared(Queue<int>());
    toggle->push(true);
    toggle->push(true);
    series->push(5);
    auto reactor = Switch(toggle, series);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(reactor.commit(1) == State::NONE);
  }

  TEST_CASE("continuing_series") {
    auto series = Shared(Queue<int>());
    series->push(1);
    series->push(2);
    auto reactor = Switch(Constant(true), series);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(reactor.eval() == 1);
    REQUIRE(reactor.commit(1) == State::EVALUATED);
    REQUIRE(reactor.eval() == 2);
  }

  TEST_CASE("series_without_a_value") {
    auto reactor = switch_(Constant(true), perpetual());
    REQUIRE(decltype(reactor)::is_noexcept);
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    reactor.eval();
  }

  TEST_CASE("series_completing_while_the_toggle_continues") {
    auto toggle = Shared(Queue<bool>());
    toggle->push(true);
    toggle->push(true);
    auto reactor = Switch(toggle, Constant(10));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == 10);
  }

  TEST_CASE("releasing_a_completed_toggle") {
    auto toggle = TrackedReactor<bool>(true, State::COMPLETE_EVALUATED);
    auto token = toggle.get_token();
    auto series = Shared(Queue<int>());
    auto reactor = switch_(std::move(toggle), series);
    series->push(5);
    REQUIRE(reactor.commit(0) == State::EVALUATED);
    REQUIRE(reactor.eval() == 5);
    REQUIRE(token.expired());
  }
}

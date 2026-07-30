#include <optional>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Cell.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Weak.hpp"

using namespace Aspen;

TEST_SUITE("Weak") {
  TEST_CASE("weak_queue") {
    auto s1 = std::optional<Shared<Queue<int>>>();
    s1.emplace();
    auto s2 = Weak(*s1);
    (*s1)->push(5);
    (*s1)->push(10);
    (*s1)->push(15);
    REQUIRE(s2.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(s2.eval() == 5);
    REQUIRE(s2.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(s2.eval() == 10);
    s1 = std::nullopt;
    REQUIRE(s2.commit(2) == State::COMPLETE);
    REQUIRE(s2.eval() == 10);
  }

  TEST_CASE("weak_box") {
    auto s1 = std::optional<Shared<Queue<int>>>();
    s1.emplace();
    auto s2 = std::optional<Shared<Box<int>>>(*s1);
    auto s3 = Weak(*s2);
    (*s1)->push(5);
    (*s1)->push(10);
    (*s1)->push(15);
    REQUIRE(s3.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(s3.eval() == 5);
    REQUIRE(s3.commit(1) == State::CONTINUE_EVALUATED);
    REQUIRE(s3.eval() == 10);
    s1 = std::nullopt;
    s2 = std::nullopt;
    REQUIRE(s3.commit(2) == State::COMPLETE);
    REQUIRE(s3.eval() == 10);
  }

  TEST_CASE("a_reactor_that_cannot_throw") {
    auto shared = std::optional(Shared(Cell(1)));
    auto weak = Weak(*shared);
    REQUIRE(decltype(weak)::is_noexcept);
    REQUIRE(weak.commit(0) == State::EVALUATED);
    REQUIRE(weak.eval() == 1);
    (*shared)->set(2);
    REQUIRE(weak.commit(1) == State::EVALUATED);
    REQUIRE(weak.eval() == 2);
    shared = std::nullopt;
    REQUIRE(weak.commit(2) == State::COMPLETE);
    REQUIRE(weak.eval() == 2);
  }

  TEST_CASE("locking_a_weak") {
    auto shared = std::optional(Shared(Queue<int>()));
    auto weak = Weak(*shared);
    auto locked = weak.lock();
    REQUIRE(locked);
    (*locked)->push(5);
    REQUIRE(weak.commit(0) == State::EVALUATED);
    REQUIRE(weak.eval() == 5);
    locked = std::nullopt;
    shared = std::nullopt;
    REQUIRE(!weak.lock());
  }

  TEST_CASE("copying_and_moving_a_weak") {
    auto shared = std::optional(Shared(Queue<int>()));
    auto weak = Weak(*shared);
    (*shared)->push(5);
    auto copy = weak;
    REQUIRE(copy.commit(0) == State::EVALUATED);
    REQUIRE(copy.eval() == 5);
    auto moved = std::move(copy);
    (*shared)->push(10);
    REQUIRE(moved.commit(1) == State::EVALUATED);
    REQUIRE(moved.eval() == 10);
    REQUIRE(moved.lock());
  }
}

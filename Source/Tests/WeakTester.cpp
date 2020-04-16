#include <doctest/doctest.h>
#include <optional>
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
}

#include <catch2/catch.hpp>
#include <optional>
#include "Aspen/Constant.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Weak.hpp"

using namespace Aspen;

TEST_CASE("test_weak_queue", "[Weak]") {
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

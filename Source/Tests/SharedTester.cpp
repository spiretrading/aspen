#include <doctest/doctest.h>
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Shared") {
  TEST_CASE("shared_chain") {
    auto s1 = Shared(Chain(constant(10), constant(9)));
    auto s2 = s1;
    REQUIRE(s1.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(s1.eval() == 10);
    REQUIRE(s2.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(s2.eval() == 10);
    REQUIRE(s1.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(s1.eval() == 9);
    REQUIRE(s2.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(s2.eval() == 9);
  }

  TEST_CASE("shared_from_unique") {
    auto c = Unique(std::make_unique<Constant<int>>(5));
    auto s = Shared(std::move(c));
    REQUIRE(s.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(s.eval() == 5);
  }

  TEST_CASE("shared_constant_to_shared_box") {
    auto c = Shared(Constant(123));
    auto b = Shared<Box<int>>(c);
    REQUIRE(b.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(b.eval() == 123);
  }

  TEST_CASE("shared_with_none") {
    auto a = Shared(chain(123, none<int>(), 321, none<int>()));
    auto b = Shared(a);
    a.commit(0);
    REQUIRE(b.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE(b.eval() == 123);
    a.commit(1);
    a.commit(2);
    a.commit(3);
    REQUIRE(b.commit(3) == State::COMPLETE_EVALUATED);
    REQUIRE(b.eval() == 321);
  }
}

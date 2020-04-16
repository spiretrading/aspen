#include <string>
#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"

using namespace Aspen;
using namespace std::string_literals;

TEST_SUITE("Constant") {
  TEST_CASE("constant_int") {
    auto constant = Constant(123);
    REQUIRE(constant.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(constant.eval() == 123);
  }

  TEST_CASE("constant_float") {
    auto constant = Constant(3.14);
    REQUIRE(constant.commit(100) == State::COMPLETE_EVALUATED);
    REQUIRE(constant.eval() == 3.14);
  }

  TEST_CASE("constant_string") {
    auto constant = Constant("hello world"s);
    REQUIRE(constant.commit(123) == State::COMPLETE_EVALUATED);
    REQUIRE(constant.eval() == "hello world"s);
  }
}

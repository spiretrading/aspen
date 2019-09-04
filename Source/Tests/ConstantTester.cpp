#include <string>
#include <catch2/catch.hpp>
#include "Aspen/Constant.hpp"

using namespace Aspen;
using namespace std::string_literals;

TEST_CASE("test_constant_int", "[Constant]") {
  auto constant = Constant(123);
  REQUIRE(constant.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(constant.eval() == 123);
}

TEST_CASE("test_constant_float", "[Constant]") {
  auto constant = Constant(3.14);
  REQUIRE(constant.commit(100) == State::COMPLETE_EVALUATED);
  REQUIRE(constant.eval() == 3.14);
}

TEST_CASE("test_constant_string", "[Constant]") {
  auto constant = Constant("hello world"s);
  REQUIRE(constant.commit(123) == State::COMPLETE_EVALUATED);
  REQUIRE(constant.eval() == "hello world"s);
}

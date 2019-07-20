#include <catch2/catch.hpp>
#include "Aspen/Box.hpp"
#include "Aspen/Constant.hpp"

using namespace Aspen;

TEST_CASE("test_constant_box", "[Box]") {
  auto constant = Constant(123);
  auto box = Box(std::move(constant));
  REQUIRE(box.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(box.eval() == 123);
}

TEST_CASE("test_pointer_box", "[Box]") {
  auto constant = Constant(123);
  auto box = Box(&constant);
  REQUIRE(box.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(box.eval() == 123);
}

TEST_CASE("test_void_box", "[Box]") {
  auto box = Box<void>(Constant(123));
  REQUIRE(box.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE_NOTHROW(box.eval());
}

#include <doctest/doctest.h>

import <utility>;
import Aspen;

using namespace Aspen;

TEST_SUITE("Box") {
  TEST_CASE("constant_box") {
    auto constant = Constant(123);
    auto box = Box(std::move(constant));
    REQUIRE(box.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(box.eval() == 123);
  }

  TEST_CASE("pointer_box") {
    auto box = Box(Constant(123));
    REQUIRE(box.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(box.eval() == 123);
  }

  TEST_CASE("void_box") {
    auto box = Box<void>(Constant(123));
    REQUIRE(box.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE_NOTHROW(box.eval());
  }
}

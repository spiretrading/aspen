#include <stdexcept>
#include <vector>
#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("VectorSync") {
  TEST_CASE("empty_vector_sync") {
    auto list = std::vector<int>();
    auto reactors = std::vector<Box<int>>();
    reactors.push_back(box(constant(5)));
    reactors.push_back(box(none<int>()));
    reactors.push_back(box(constant(10)));
    auto reactor = VectorSync(list, std::move(reactors));
    REQUIRE(reactor.commit(0) == State::COMPLETE);
    REQUIRE(reactor.eval() == std::vector{5, 0, 0});
    REQUIRE(list == std::vector{5, 0, 0});
  }

  TEST_CASE("single_vector_sync") {
    auto list = std::vector<int>();
    auto reactors = std::vector<Box<int>>();
    reactors.push_back(box(constant(5)));
    reactors.push_back(box(constant(12)));
    reactors.push_back(box(constant(3)));
    auto reactor = VectorSync(list, std::move(reactors));
    REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == std::vector{5, 12, 3});
    REQUIRE(list == std::vector{5, 12, 3});
  }

  TEST_CASE("exception_vector_sync") {
    auto list = std::vector<int>();
    auto reactors = std::vector<Box<int>>();
    reactors.push_back(box(constant(5)));
    reactors.push_back(box(chain(throws<int>(std::runtime_error("fail")),
      constant(12))));
    reactors.push_back(box(constant(3)));
    auto reactor = VectorSync(list, std::move(reactors));
    REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
    REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
    REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
    REQUIRE(reactor.eval() == std::vector{5, 12, 3});
    REQUIRE(list == std::vector{5, 12, 3});
  }
}

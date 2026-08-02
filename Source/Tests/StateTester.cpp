#include <sstream>
#include <string>
#include <doctest/doctest.h>
#include "Aspen/State.hpp"

using namespace Aspen;

namespace {
  std::string to_string(State state) {
    auto sink = std::ostringstream();
    sink << state;
    return sink.str();
  }

  constexpr State merge(State left, State right) {
    return static_cast<State>(
      static_cast<unsigned char>(left) | static_cast<unsigned char>(right));
  }
}

TEST_SUITE("State") {
  TEST_CASE("combine_function") {
    REQUIRE(combine(State::NONE, State::EVALUATED) == State::EVALUATED);
    REQUIRE(combine(State::EVALUATED, State::CONTINUE) ==
      State::CONTINUE_EVALUATED);
    REQUIRE(combine(State::EVALUATED, State::COMPLETE) ==
      State::COMPLETE_EVALUATED);
    REQUIRE(combine(State::EVALUATED, State::EVALUATED) == State::EVALUATED);
  }

  TEST_CASE("reset_function") {
    REQUIRE(reset(State::COMPLETE_EVALUATED, State::COMPLETE) ==
      State::EVALUATED);
    REQUIRE(reset(State::COMPLETE_EVALUATED, State::CONTINUE) ==
      State::COMPLETE_EVALUATED);
    REQUIRE(reset(State::EVALUATED, State::EVALUATED) == State::NONE);
    REQUIRE(reset(State::NONE, State::COMPLETE) == State::NONE);
    REQUIRE(reset(State::CONTINUE_EVALUATED, State::COMPLETE_EVALUATED) ==
      State::CONTINUE);
  }

  TEST_CASE("predicates") {
    REQUIRE(!has_evaluation(State::NONE));
    REQUIRE(has_evaluation(State::EVALUATED));
    REQUIRE(has_evaluation(State::CONTINUE_EVALUATED));
    REQUIRE(has_evaluation(State::COMPLETE_EVALUATED));
    REQUIRE(!has_evaluation(State::CONTINUE));
    REQUIRE(!has_evaluation(State::COMPLETE));
    REQUIRE(!has_continuation(State::NONE));
    REQUIRE(has_continuation(State::CONTINUE));
    REQUIRE(has_continuation(State::CONTINUE_EVALUATED));
    REQUIRE(!has_continuation(State::EVALUATED));
    REQUIRE(!has_continuation(State::COMPLETE_EVALUATED));
    REQUIRE(!is_complete(State::NONE));
    REQUIRE(is_complete(State::COMPLETE));
    REQUIRE(is_complete(State::COMPLETE_EVALUATED));
    REQUIRE(!is_complete(State::EVALUATED));
    REQUIRE(!is_complete(State::CONTINUE_EVALUATED));
  }

  TEST_CASE("streaming") {
    REQUIRE(to_string(State::NONE) == "NONE");
    REQUIRE(to_string(State::EVALUATED) == "EVALUATED");
    REQUIRE(to_string(State::CONTINUE) == "CONTINUE");
    REQUIRE(to_string(State::COMPLETE) == "COMPLETE");
    REQUIRE(to_string(State::CONTINUE_EVALUATED) == "CONTINUE_EVALUATED");
    REQUIRE(to_string(State::COMPLETE_EVALUATED) == "COMPLETE_EVALUATED");
  }

  TEST_CASE("streaming_a_combination") {
    REQUIRE(to_string(merge(State::COMPLETE, State::CONTINUE)) ==
      "COMPLETE_CONTINUE");
    REQUIRE(to_string(merge(State::COMPLETE_EVALUATED, State::CONTINUE)) ==
      "COMPLETE_CONTINUE_EVALUATED");
  }

  TEST_CASE("constexpr_evaluation") {
    static_assert(combine(State::EVALUATED, State::CONTINUE) ==
      State::CONTINUE_EVALUATED);
    static_assert(reset(State::COMPLETE_EVALUATED, State::COMPLETE) ==
      State::EVALUATED);
    static_assert(has_evaluation(State::COMPLETE_EVALUATED));
    static_assert(!has_continuation(State::COMPLETE_EVALUATED));
    static_assert(is_complete(State::COMPLETE_EVALUATED));
  }
}

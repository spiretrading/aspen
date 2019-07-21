#include <catch2/catch.hpp>
#include "Aspen/CommitReactor.hpp"

using namespace Aspen;

TEST_CASE("test_empty_commit", "[CommitReactor]") {
  auto reactor = CommitReactor({});
  REQUIRE(reactor.commit(0) == State::COMPLETE_EMPTY);
  REQUIRE(reactor.eval() == State::COMPLETE_EMPTY);
}

#include <catch2/catch.hpp>
#include "Aspen/CommitHandler.hpp"

using namespace Aspen;

TEST_CASE("test_empty_commit", "[CommitHandler]") {
  auto reactor = CommitHandler({});
  REQUIRE(reactor.commit(0) == State::COMPLETE_EMPTY);
}

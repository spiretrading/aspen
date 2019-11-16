#include <catch2/catch.hpp>
#include "Aspen/Chain.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Sync.hpp"
#include "Aspen/Throw.hpp"
#include "Aspen/VectorSync.hpp"

using namespace Aspen;

TEST_CASE("test_empty_vector_sync", "[VectorSync]") {
  auto list = std::vector<int>();
  auto reactor = VectorSync(list,
    std::vector{box(constant(5)), box(none<int>()), box(constant(10))});
  REQUIRE(reactor.commit(0) == State::COMPLETE);
  REQUIRE(reactor.eval() == std::vector{5, 0, 0});
  REQUIRE(list == std::vector{5, 0, 0});
}

TEST_CASE("test_single_vector_sync", "[VectorSync]") {
  auto list = std::vector<int>();
  auto reactor = VectorSync(list, std::vector{box(constant(5)),
    box(constant(12)), box(constant(3))});
  REQUIRE(reactor.commit(0) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == std::vector{5, 12, 3});
  REQUIRE(list == std::vector{5, 12, 3});
}

TEST_CASE("test_exception_vector_sync", "[VectorSync]") {
  auto list = std::vector<int>();
  auto reactor = VectorSync(list, std::vector{box(constant(5)),
    box(chain(throws<int>(std::runtime_error("fail")), constant(12))),
    box(constant(3))});
  REQUIRE(reactor.commit(0) == State::CONTINUE_EVALUATED);
  REQUIRE_THROWS_AS(reactor.eval(), std::runtime_error);
  REQUIRE(reactor.commit(1) == State::COMPLETE_EVALUATED);
  REQUIRE(reactor.eval() == std::vector{5, 12, 3});
  REQUIRE(list == std::vector{5, 12, 3});
}

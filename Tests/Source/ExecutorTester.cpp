#include <optional>
#include <thread>
#include <vector>
#include <catch2/catch.hpp>
#include "Aspen/Constant.hpp"
#include "Aspen/Executor.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"

using namespace Aspen;

TEST_CASE("test_run_until_none_empty", "[Executor]") {
  auto result = std::optional<int>();
  auto executor = Executor(
    lift([&] (const auto& value) {
      result = value;
    }, none<int>()));
  executor.run_until_none();
  REQUIRE(!result.has_value());
}

TEST_CASE("test_run_until_none_constant", "[Executor]") {
  auto result = std::optional<int>();
  auto executor = Executor(
    lift([&] (const auto& value) {
      result = value;
    }, constant(5)));
  executor.run_until_none();
  REQUIRE(result.has_value());
  REQUIRE(*result == 5);
}

TEST_CASE("test_run_until_complete", "[Executor]") {
  auto queue = Queue<int>();
  auto results = std::vector<int>();
  auto executor = Executor(
    lift([&] (const auto& value) {
      results.push_back(value);
    }, &queue));
  auto executor_thread = std::thread([&] {
    executor.run_until_complete();
  });
  queue.push(10);
  queue.push(20);
  queue.push(30);
  queue.push(40);
  queue.set_complete(100);
  executor_thread.join();
  REQUIRE(results.size() == 5);
  REQUIRE(results == std::vector{10, 20, 30, 40, 100});
}

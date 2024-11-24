#include <optional>
#include <thread>
#include <vector>
#include <doctest/doctest.h>

import Aspen;

using namespace Aspen;

TEST_SUITE("Executor") {
  TEST_CASE("run_until_none_empty") {
    auto result = std::optional<int>();
    auto executor = Executor(
      lift([&] (const auto& value) {
        result = value;
      }, none<int>()));
    executor.run_until_none();
    REQUIRE(!result.has_value());
  }

  TEST_CASE("run_until_none_constant") {
    auto result = std::optional<int>();
    auto executor = Executor(
      lift([&] (const auto& value) {
        result = value;
      }, constant(5)));
    executor.run_until_none();
    REQUIRE(result.has_value());
    REQUIRE(*result == 5);
  }

  TEST_CASE("run_until_complete") {
    auto queue = Shared(Queue<int>());
    auto results = std::vector<int>();
    auto executor = Executor(
      lift([&] (const auto& value) {
        results.push_back(value);
      }, queue));
    auto executor_thread = std::thread([&] {
      executor.run_until_complete();
    });
    queue->push(10);
    queue->push(20);
    queue->push(30);
    queue->push(40);
    queue->set_complete(100);
    executor_thread.join();
    REQUIRE(results.size() == 5);
    REQUIRE(results == std::vector{10, 20, 30, 40, 100});
  }
}

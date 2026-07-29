#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Executor.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"

using namespace Aspen;

TEST_SUITE("Executor") {
  TEST_CASE("run_until_none_empty") {
    auto result = std::optional<int>();
    auto executor = Executor(
      lift([&] (const auto& value) {
        result = value;
      }, none<int>()));
    executor.run_until_none();
    REQUIRE(!result);
  }

  TEST_CASE("run_until_none_constant") {
    auto result = std::optional<int>();
    auto executor = Executor(
      lift([&] (const auto& value) {
        result = value;
      }, constant(5)));
    executor.run_until_none();
    REQUIRE(result);
    REQUIRE(*result == 5);
  }

  TEST_CASE("run_until_none_continues") {
    auto results = std::vector<int>();
    auto executor = Executor(
      lift([&] (const auto& value) {
        results.push_back(value);
      }, chain(1, 2, 3)));
    executor.run_until_none();
    REQUIRE(results == std::vector{1, 2, 3});
  }

  TEST_CASE("run_until_none_restores_the_trigger") {
    auto trigger = Trigger();
    Trigger::set_trigger(trigger);
    auto executor = Executor(constant(5));
    executor.run_until_none();
    REQUIRE(Trigger::get_trigger() == &trigger);
    Trigger::set_trigger(nullptr);
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

  TEST_CASE("run_until_complete_wakes_on_every_push") {
    auto queue = Shared(Queue<int>());
    auto mutex = std::mutex();
    auto condition = std::condition_variable();
    auto results = std::vector<int>();
    auto executor = Executor(
      lift([&] (const auto& value) {
        auto lock = std::lock_guard(mutex);
        results.push_back(value);
        condition.notify_all();
      }, queue));
    auto executor_thread = std::thread([&] {
      executor.run_until_complete();
    });
    auto wait_for = [&] (std::size_t count) {
      auto lock = std::unique_lock(mutex);
      condition.wait(lock, [&] {
        return results.size() >= count;
      });
    };
    queue->push(10);
    wait_for(1);
    queue->push(20);
    wait_for(2);
    queue->push(30);
    wait_for(3);
    queue->set_complete(40);
    executor_thread.join();
    REQUIRE(results == std::vector{10, 20, 30, 40});
  }

  TEST_CASE("aborting_a_waiting_executor") {
    auto queue = Shared(Queue<int>());
    auto mutex = std::mutex();
    auto condition = std::condition_variable();
    auto results = std::vector<int>();
    auto executor = Executor(
      lift([&] (const auto& value) {
        auto lock = std::lock_guard(mutex);
        results.push_back(value);
        condition.notify_all();
      }, queue));
    auto executor_thread = std::thread([&] {
      executor.run_until_complete();
    });
    queue->push(10);
    {
      auto lock = std::unique_lock(mutex);
      condition.wait(lock, [&] {
        return !results.empty();
      });
    }
    executor.abort();
    executor_thread.join();
    REQUIRE(results == std::vector{10});
  }

  TEST_CASE("aborting_a_reactor_that_always_continues") {
    auto counter = std::make_shared<std::atomic_int>(0);
    auto executor = Executor(
      lift([counter] (const auto& value) {
        ++*counter;
        return FunctionEvaluation<int>(value, State::CONTINUE);
      }, constant(1)));
    auto executor_thread = std::thread([&] {
      executor.run_until_complete();
    });
    while(counter->load() < 10) {}
    executor.abort();
    executor_thread.join();
    REQUIRE(counter->load() >= 10);
  }

  TEST_CASE("aborting_before_a_run_starts") {
    auto counter = std::make_shared<std::atomic_int>(0);
    auto executor = Executor(
      lift([counter] (const auto& value) {
        ++*counter;
      }, constant(1)));
    executor.abort();
    executor.run_until_complete();
    REQUIRE(counter->load() == 0);
  }
}

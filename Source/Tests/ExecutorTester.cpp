#include <atomic>
#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/Chain.hpp"
#include "Aspen/Constant.hpp"
#include "Aspen/Executor.hpp"
#include "Aspen/Lift.hpp"
#include "Aspen/None.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/Traits.hpp"
#include "Aspen/Trigger.hpp"

using namespace Aspen;

namespace {
  struct Counter {
    using Type = int;
    std::shared_ptr<int> m_commits;

    Counter()
      : m_commits(std::make_shared<int>(0)) {}

    State commit(std::uint64_t sequence) noexcept {
      ++*m_commits;
      return State::COMPLETE_EVALUATED;
    }

    const int& eval() const noexcept {
      return *m_commits;
    }
  };

  struct Idler {
    using Type = int;
    std::shared_ptr<int> m_commits;

    Idler()
      : m_commits(std::make_shared<int>(0)) {}

    State commit(std::uint64_t sequence) noexcept {
      ++*m_commits;
      return State::EVALUATED;
    }

    const int& eval() const noexcept {
      return *m_commits;
    }
  };

  template<IsReactor R>
  struct Tally {
    using Type = reactor_result_t<R>;
    static constexpr auto is_noexcept = is_noexcept_reactor_v<R>;
    R m_reactor;
    std::shared_ptr<std::atomic_int> m_commits;

    Tally(R reactor, std::shared_ptr<std::atomic_int> commits)
      : m_reactor(std::move(reactor)),
        m_commits(std::move(commits)) {}

    State commit(std::uint64_t sequence) noexcept {
      m_commits->fetch_add(1);
      return m_reactor.commit(sequence);
    }

    decltype(auto) eval() const noexcept(is_noexcept) {
      return m_reactor.eval();
    }
  };
}

TEST_SUITE("Executor") {
  TEST_CASE("run_until_none_with_no_update") {
    auto result = std::optional<int>();
    auto executor = Executor(
      lift([&] (const auto& value) {
        result = value;
      }, none<int>()));
    executor.run_until_none();
    REQUIRE(!result);
  }

  TEST_CASE("run_until_none_with_a_constant") {
    auto result = std::optional<int>();
    auto executor = Executor(
      lift([&] (const auto& value) {
        result = value;
      }, constant(5)));
    executor.run_until_none();
    REQUIRE(result);
    REQUIRE(*result == 5);
  }

  TEST_CASE("run_until_none_with_a_continuation") {
    auto results = std::vector<int>();
    auto executor = Executor(
      lift([&] (const auto& value) {
        results.push_back(value);
      }, chain(1, 2, 3)));
    executor.run_until_none();
    REQUIRE(results == std::vector{1, 2, 3});
  }

  TEST_CASE("run_until_none_restores_the_trigger") {
    auto previous = Trigger::get_trigger();
    auto trigger = Trigger();
    Trigger::set_trigger(trigger);
    auto executor = Executor(constant(5));
    executor.run_until_none();
    REQUIRE(Trigger::get_trigger() == &trigger);
    Trigger::set_trigger(previous);
  }

  TEST_CASE("run_until_complete_restores_the_trigger") {
    auto previous = Trigger::get_trigger();
    auto trigger = Trigger();
    Trigger::set_trigger(trigger);
    auto executor = Executor(constant(5));
    executor.run_until_complete();
    REQUIRE(Trigger::get_trigger() == &trigger);
    Trigger::set_trigger(previous);
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

  TEST_CASE("run_until_complete_with_a_queue") {
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

  TEST_CASE("run_until_complete_with_a_cell") {
    auto cell = Shared(Cell(0));
    auto mutex = std::mutex();
    auto condition = std::condition_variable();
    auto results = std::vector<int>();
    auto executor = Executor(
      lift([&] (int value) {
        auto lock = std::lock_guard(mutex);
        results.push_back(value);
        condition.notify_all();
      }, cell));
    auto executor_thread = std::thread([&] {
      executor.run_until_complete();
    });
    auto wait_for = [&] (std::size_t count) {
      auto lock = std::unique_lock(mutex);
      condition.wait(lock, [&] {
        return results.size() >= count;
      });
    };
    wait_for(1);
    cell->set(1);
    wait_for(2);
    cell->set(2);
    wait_for(3);
    cell->set_complete(3);
    executor_thread.join();
    REQUIRE(results == std::vector{0, 1, 2, 3});
  }

  TEST_CASE("waiting_after_a_continuation") {
    auto queue = Shared(Queue<int>());
    auto commits = std::make_shared<std::atomic_int>(0);
    auto mutex = std::mutex();
    auto condition = std::condition_variable();
    auto results = std::vector<int>();
    queue->push(10);
    queue->push(20);
    auto executor = Executor(Tally(lift([&] (int value) {
      auto lock = std::lock_guard(mutex);
      results.push_back(value);
      condition.notify_all();
    }, queue), commits));
    auto executor_thread = std::thread([&] {
      executor.run_until_complete();
    });
    {
      auto lock = std::unique_lock(mutex);
      condition.wait(lock, [&] {
        return results.size() == 2;
      });
    }
    queue->set_complete();
    executor_thread.join();
    REQUIRE(results == std::vector{10, 20});
    REQUIRE(commits->load() == 3);
  }

  TEST_CASE("aborting_while_waiting") {
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

  TEST_CASE("aborting_a_continuation") {
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
    auto stopped = counter->load();
    executor.run_until_complete();
    REQUIRE(counter->load() == stopped);
  }

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  TEST_CASE("resuming_after_an_interrupt") {
    auto counter = std::make_shared<std::atomic_int>(0);
    auto is_continuing = std::make_shared<std::atomic_bool>(true);
    auto executor = Executor(
      lift([counter, is_continuing] (const auto& value) {
        ++*counter;
        if(is_continuing->load()) {
          return FunctionEvaluation<int>(value, State::CONTINUE);
        }
        return FunctionEvaluation<int>(value, State::COMPLETE);
      }, constant(1)));
    auto executor_thread = std::thread([&] {
      executor.run_until_complete();
    });
    while(counter->load() < 10) {}
    std::raise(SIGINT);
    executor_thread.join();
    auto stopped = counter->load();
    is_continuing->store(false);
    executor.run_until_none();
    REQUIRE(counter->load() > stopped);
  }
#endif

  TEST_CASE("aborting_before_a_run") {
    auto counter = std::make_shared<std::atomic_int>(0);
    auto executor = Executor(
      lift([counter] (const auto& value) {
        ++*counter;
      }, constant(1)));
    executor.abort();
    executor.run_until_complete();
    REQUIRE(counter->load() == 0);
  }

  TEST_CASE("run_until_none_twice") {
    auto counter = Counter();
    auto executor = Executor(counter);
    executor.run_until_none();
    REQUIRE(*counter.m_commits == 1);
    executor.run_until_none();
    REQUIRE(*counter.m_commits == 1);
  }

  TEST_CASE("resuming_without_an_update") {
    auto idler = Idler();
    auto executor = Executor(idler);
    executor.run_until_none();
    REQUIRE(*idler.m_commits == 1);
    executor.run_until_none();
    REQUIRE(*idler.m_commits == 1);
  }

  TEST_CASE("run_until_complete_twice") {
    auto counter = Counter();
    auto executor = Executor(counter);
    executor.run_until_complete();
    REQUIRE(*counter.m_commits == 1);
    executor.run_until_complete();
    REQUIRE(*counter.m_commits == 1);
  }

  TEST_CASE("run_until_none_after_aborting") {
    auto counter = Counter();
    auto executor = Executor(counter);
    executor.abort();
    executor.run_until_none();
    REQUIRE(*counter.m_commits == 0);
  }

  TEST_CASE("aborting_an_overlapping_run") {
    auto first_queue = Shared(Queue<int>());
    auto second_queue = Shared(Queue<int>());
    auto mutex = std::mutex();
    auto condition = std::condition_variable();
    auto first_values = std::vector<int>();
    auto second_values = std::vector<int>();
    auto first = Executor(lift([&] (int value) {
      auto lock = std::lock_guard(mutex);
      first_values.push_back(value);
      condition.notify_all();
    }, first_queue));
    auto second = Executor(lift([&] (int value) {
      auto lock = std::lock_guard(mutex);
      second_values.push_back(value);
      condition.notify_all();
    }, second_queue));
    auto first_thread = std::thread([&] {
      first.run_until_complete();
    });
    auto second_thread = std::thread([&] {
      second.run_until_complete();
    });
    second_queue->push(2);
    {
      auto lock = std::unique_lock(mutex);
      condition.wait(lock, [&] {
        return !second_values.empty();
      });
    }
    first_queue->set_complete(1);
    first_thread.join();
    second.abort();
    second_thread.join();
    REQUIRE(first_values == std::vector{1});
    REQUIRE(second_values == std::vector{2});
  }
}

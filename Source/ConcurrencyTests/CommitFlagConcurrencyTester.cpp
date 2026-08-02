#include <array>
#include <atomic>
#include <barrier>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <latch>
#include <memory>
#include <optional>
#include <semaphore>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include "Aspen/Cell.hpp"
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Trigger.hpp"

using namespace Aspen;

namespace {
  constexpr auto HOLDERS = std::size_t(8);
  constexpr auto PARENTS = std::size_t(16);
  constexpr auto PUSHES = 1000;
  constexpr auto RAISES = std::size_t(100000);
  constexpr auto ROUNDS = std::size_t(1000);
  constexpr auto TIMEOUT = std::chrono::seconds(30);
  constexpr auto DELAY = std::chrono::milliseconds(20);

  int get_iterations() {
    auto variable = std::getenv("ASPEN_CONCURRENCY_ITERATIONS");
    if(!variable) {
      return 1000;
    }
    auto count = std::atoi(variable);
    if(count <= 0) {
      return 1000;
    }
    return count;
  }
}

TEST_SUITE("CommitFlagConcurrency") {
  TEST_CASE("parent_assignment") {
    auto iterations = get_iterations();
    for(auto iteration = 0; iteration != iterations; ++iteration) {
      auto flags = std::deque<std::unique_ptr<CommitFlag>>();
      auto parents = std::deque<std::unique_ptr<CommitFlag>>();
      for(auto i = std::size_t(0); i != ROUNDS; ++i) {
        flags.push_back(std::make_unique<CommitFlag>());
        flags.back()->clear();
        parents.push_back(std::make_unique<CommitFlag>());
        parents.back()->clear();
      }
      auto observed = std::vector<char>(ROUNDS);
      auto start = std::barrier(2);
      auto producer = std::thread([&] {
        for(auto i = std::size_t(0); i != ROUNDS; ++i) {
          start.arrive_and_wait();
          flags[i]->raise();
        }
      });
      for(auto i = std::size_t(0); i != ROUNDS; ++i) {
        start.arrive_and_wait();
        flags[i]->set_parent(parents[i].get());
        observed[i] = flags[i]->is_raised();
      }
      producer.join();
      auto lost = std::size_t(0);
      for(auto i = std::size_t(0); i != ROUNDS; ++i) {
        if(!observed[i] && !parents[i]->is_raised()) {
          ++lost;
        }
      }
      REQUIRE(lost == 0);
    }
  }

  TEST_CASE("parent_registration") {
    auto iterations = get_iterations();
    for(auto iteration = 0; iteration != iterations; ++iteration) {
      auto flags = std::deque<std::unique_ptr<CommitFlag>>();
      auto parents = std::deque<std::unique_ptr<CommitFlag>>();
      for(auto i = std::size_t(0); i != ROUNDS; ++i) {
        flags.push_back(std::make_unique<CommitFlag>());
        flags.back()->clear();
        parents.push_back(std::make_unique<CommitFlag>());
        parents.back()->clear();
      }
      auto start = std::barrier(2);
      auto producer = std::thread([&] {
        for(auto i = std::size_t(0); i != ROUNDS; ++i) {
          start.arrive_and_wait();
          flags[i]->raise();
        }
      });
      for(auto i = std::size_t(0); i != ROUNDS; ++i) {
        start.arrive_and_wait();
        flags[i]->add_parent(*parents[i]);
      }
      producer.join();
      auto missed = std::size_t(0);
      for(auto i = std::size_t(0); i != ROUNDS; ++i) {
        if(!parents[i]->is_raised()) {
          ++missed;
        }
      }
      REQUIRE(missed == 0);
    }
  }

  TEST_CASE("parent_removal") {
    auto iterations = get_iterations();
    for(auto iteration = 0; iteration != iterations; ++iteration) {
      auto flags = std::deque<std::unique_ptr<CommitFlag>>();
      auto parents = std::deque<std::unique_ptr<CommitFlag>>();
      for(auto i = std::size_t(0); i != ROUNDS; ++i) {
        flags.push_back(std::make_unique<CommitFlag>());
        parents.push_back(std::make_unique<CommitFlag>());
        flags.back()->add_parent(*parents.back());
        flags.back()->clear();
        parents.back()->clear();
      }
      auto start = std::barrier(2);
      auto producer = std::thread([&] {
        for(auto i = std::size_t(0); i != ROUNDS; ++i) {
          start.arrive_and_wait();
          flags[i]->raise();
        }
      });
      for(auto i = std::size_t(0); i != ROUNDS; ++i) {
        start.arrive_and_wait();
        flags[i]->remove_parent(*parents[i]);
        parents[i]->clear();
      }
      producer.join();
      auto stale = std::size_t(0);
      for(auto i = std::size_t(0); i != ROUNDS; ++i) {
        if(parents[i]->is_raised()) {
          ++stale;
        }
      }
      REQUIRE(stale == 0);
    }
  }

  TEST_CASE("parent_destruction") {
    auto iterations = get_iterations();
    for(auto iteration = 0; iteration != iterations; ++iteration) {
      auto source = Shared(Cell<int>(0));
      auto entered = std::binary_semaphore(0);
      auto trigger = Trigger([&] {
        entered.release();
        std::this_thread::sleep_for(DELAY);
      });
      auto word = std::atomic_uint64_t(0);
      auto parents = std::deque<std::unique_ptr<CommitFlag>>();
      auto holders = std::deque<std::optional<Shared<Cell<int>>>>();
      for(auto i = std::size_t(0); i != PARENTS; ++i) {
        parents.push_back(std::make_unique<CommitFlag>());
        holders.emplace_back(source);
        auto scope = CommitFlagScope(*parents[i]);
        (*holders.back()).commit(0);
      }
      for(auto& parent : parents) {
        parent->clear();
      }
      parents[1]->set_trigger(&trigger);
      parents[PARENTS - 1]->set_slot(&word, 0);
      auto producer = std::thread([&] {
        source->set(1);
      });
      auto is_entered = entered.try_acquire_for(TIMEOUT);
      holders[PARENTS - 1].reset();
      auto is_reached = word.load() != 0;
      parents[PARENTS - 1].reset();
      producer.join();
      REQUIRE(is_entered);
      REQUIRE(is_reached);
    }
  }

  TEST_CASE("parent_list_mutation") {
    auto iterations = get_iterations();
    auto expected = std::vector<int>();
    for(auto i = 1; i <= PUSHES; ++i) {
      expected.push_back(i);
    }
    for(auto iteration = 0; iteration != iterations; ++iteration) {
      auto reactor = Shared(Queue<int>());
      auto& queue = *reactor;
      auto word = std::atomic_uint64_t(0);
      auto parents = std::deque<std::unique_ptr<CommitFlag>>();
      auto holders = std::deque<Shared<Queue<int>>>();
      auto sequence = std::uint64_t(0);
      reactor.commit(sequence);
      ++sequence;
      auto start = std::latch(2);
      auto producer = std::thread([&] {
        start.arrive_and_wait();
        for(auto i = 1; i <= PUSHES; ++i) {
          queue.push(i);
        }
        queue.set_complete();
      });
      start.arrive_and_wait();
      auto values = std::vector<int>();
      auto state = State::NONE;
      auto deadline = std::chrono::steady_clock::now() + TIMEOUT;
      while(!is_complete(state) &&
          std::chrono::steady_clock::now() < deadline) {
        state = reactor.commit(sequence);
        if(has_evaluation(state)) {
          values.push_back(reactor.eval());
        }
        parents.push_back(std::make_unique<CommitFlag>());
        parents.back()->clear();
        parents.back()->set_slot(
          &word, static_cast<std::uint8_t>(sequence % PARENTS));
        holders.emplace_back(reactor);
        {
          auto scope = CommitFlagScope(*parents.back());
          holders.back().commit(sequence);
        }
        if(holders.size() > HOLDERS) {
          holders.pop_front();
          parents.pop_front();
        }
        ++sequence;
      }
      producer.join();
      REQUIRE(is_complete(state));
      REQUIRE(values == expected);
    }
  }
}

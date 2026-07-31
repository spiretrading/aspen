#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <latch>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/State.hpp"

using namespace Aspen;

namespace {
  constexpr auto HOLDERS = std::size_t(8);
  constexpr auto PARENTS = std::size_t(16);
  constexpr auto PUSHES = 1000;
  constexpr auto RAISES = std::size_t(100000);
  constexpr auto TIMEOUT = std::chrono::seconds(30);

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
      auto parents = std::array<CommitFlag, 2>();
      auto flag = CommitFlag();
      auto start = std::latch(2);
      auto producer = std::thread([&] {
        start.arrive_and_wait();
        for(auto i = std::size_t(0); i != RAISES; ++i) {
          flag.raise();
        }
      });
      start.arrive_and_wait();
      for(auto i = std::size_t(0); i != RAISES; ++i) {
        flag.clear();
        flag.set_parent(&parents[i % 2]);
      }
      producer.join();
      parents[0].clear();
      flag.clear();
      flag.set_parent(&parents[0]);
      flag.raise();
      REQUIRE(parents[0].is_raised());
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
      auto parents = std::array<CommitFlag, PARENTS>();
      for(auto i = std::size_t(0); i != PARENTS; ++i) {
        parents[i].clear();
        parents[i].set_slot(&word, static_cast<std::uint8_t>(i));
      }
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
        holders.emplace_back(reactor);
        {
          auto scope = CommitFlagScope(parents[sequence % PARENTS]);
          holders.back().commit(sequence);
        }
        if(holders.size() > HOLDERS) {
          holders.pop_front();
        }
        ++sequence;
      }
      producer.join();
      REQUIRE(is_complete(state));
      REQUIRE(values == expected);
      REQUIRE(word.load() != 0);
    }
  }
}

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <latch>
#include <thread>
#include <vector>
#include <doctest/doctest.h>
#include "Aspen/Box.hpp"
#include "Aspen/Concur.hpp"
#include "Aspen/Queue.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/State.hpp"

using namespace Aspen;

namespace {
  constexpr auto CHILDREN = 8;
  constexpr auto VALUES = 200;
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

TEST_SUITE("ConcurConcurrency") {
  TEST_CASE("completed_child_removal") {
    auto iterations = get_iterations();
    auto expected = std::vector<int>();
    for(auto i = 0; i != CHILDREN; ++i) {
      for(auto j = 1; j <= VALUES; ++j) {
        expected.push_back(i * VALUES + j);
      }
    }
    for(auto iteration = 0; iteration != iterations; ++iteration) {
      auto producer = Shared(Queue<SharedBox<int>>());
      auto children = std::vector<Shared<Queue<int>>>();
      for(auto i = 0; i != CHILDREN; ++i) {
        children.push_back(Shared(Queue<int>()));
        producer->push(shared_box(children.back()));
      }
      producer->set_complete();
      auto reactor = concur(producer);
      auto start = std::latch(CHILDREN + 1);
      auto threads = std::vector<std::thread>();
      for(auto i = 0; i != CHILDREN; ++i) {
        threads.emplace_back([&, i] {
          auto& queue = *children[i];
          start.arrive_and_wait();
          for(auto j = 1; j <= VALUES; ++j) {
            queue.push(i * VALUES + j);
          }
          queue.set_complete();
        });
      }
      start.arrive_and_wait();
      auto values = std::vector<int>();
      auto state = State::NONE;
      auto sequence = std::uint64_t(0);
      auto deadline = std::chrono::steady_clock::now() + TIMEOUT;
      while(!is_complete(state) &&
          std::chrono::steady_clock::now() < deadline) {
        state = reactor.commit(sequence);
        if(has_evaluation(state)) {
          values.push_back(reactor.eval());
        }
        ++sequence;
      }
      for(auto& thread : threads) {
        thread.join();
      }
      std::sort(values.begin(), values.end());
      REQUIRE(is_complete(state));
      REQUIRE(values == expected);
    }
  }
}

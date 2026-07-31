#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <pthread.h>
#include <doctest/doctest.h>
#include "Aspen/Constant.hpp"
#include "Aspen/Executor.hpp"

using namespace Aspen;

namespace {
  using SignalAction = struct ::sigaction;

  constexpr auto DURATION = std::chrono::seconds(10);
  constexpr auto INTERVAL = std::chrono::microseconds(50);
  constexpr auto STALL = std::chrono::seconds(3);

  void ignore(int) {}
}

TEST_SUITE("ExecutorConcurrency") {
  TEST_CASE("interrupt_while_registering") {
    auto previous = SignalAction();
    auto action = SignalAction();
    action.sa_handler = &ignore;
    ::sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    ::sigaction(SIGINT, &action, &previous);
    auto stop = std::atomic_bool(false);
    auto progress = std::atomic_uint64_t(0);
    auto runner = std::thread([&] {
      while(!stop.load(std::memory_order_relaxed)) {
        auto executor = Executor(constant(1));
        executor.run_until_complete();
        progress.fetch_add(1, std::memory_order_relaxed);
      }
    });
    auto target = runner.native_handle();
    auto signaller = std::thread([&] {
      while(!stop.load(std::memory_order_relaxed)) {
        ::pthread_kill(target, SIGINT);
        std::this_thread::sleep_for(INTERVAL);
      }
    });
    auto deadline = std::chrono::steady_clock::now() + DURATION;
    auto observed = progress.load(std::memory_order_relaxed);
    auto advanced = std::chrono::steady_clock::now();
    while(std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      auto current = progress.load(std::memory_order_relaxed);
      if(current != observed) {
        observed = current;
        advanced = std::chrono::steady_clock::now();
      } else if(std::chrono::steady_clock::now() - advanced > STALL) {
        MESSAGE("stalled after " << observed << " runs");
        std::_Exit(70);
      }
    }
    stop.store(true, std::memory_order_relaxed);
    signaller.join();
    runner.join();
    ::sigaction(SIGINT, &previous, nullptr);
    REQUIRE(progress.load(std::memory_order_relaxed) != 0);
  }
}
#endif

#include <thread>
#include <doctest/doctest.h>
#include "Aspen/Trigger.hpp"

using namespace Aspen;

TEST_SUITE("Trigger") {
  TEST_CASE("signalling") {
    auto count = 0;
    auto trigger = Trigger([&] {
      ++count;
    });
    trigger.signal();
    REQUIRE(count == 1);
    trigger.signal();
    REQUIRE(count == 2);
  }

  TEST_CASE("signalling_a_default_trigger") {
    auto count = 0;
    auto trigger = Trigger();
    auto previous = Trigger::get_trigger();
    Trigger::set_trigger(trigger);
    REQUIRE(Trigger::get_trigger() == &trigger);
    trigger.signal();
    trigger.signal();
    Trigger::set_trigger(previous);
    REQUIRE(count == 0);
  }

  TEST_CASE("current_trigger") {
    auto previous = Trigger::get_trigger();
    auto trigger = Trigger();
    Trigger::set_trigger(trigger);
    REQUIRE(Trigger::get_trigger() == &trigger);
    Trigger::set_trigger(nullptr);
    REQUIRE(!Trigger::get_trigger());
    Trigger::set_trigger(&trigger);
    REQUIRE(Trigger::get_trigger() == &trigger);
    Trigger::set_trigger(previous);
  }

  TEST_CASE("current_trigger_per_thread") {
    auto previous = Trigger::get_trigger();
    auto trigger = Trigger();
    Trigger::set_trigger(trigger);
    auto observed = &trigger;
    auto observer = std::thread([&] {
      observed = Trigger::get_trigger();
    });
    observer.join();
    REQUIRE(!observed);
    REQUIRE(Trigger::get_trigger() == &trigger);
    Trigger::set_trigger(previous);
  }
}

#include <thread>
#include <doctest/doctest.h>
#include "Aspen/Trigger.hpp"

using namespace Aspen;

TEST_SUITE("Trigger") {
  TEST_CASE("signalling_a_slot") {
    auto count = 0;
    auto trigger = Trigger([&] {
      ++count;
    });
    trigger.signal();
    REQUIRE(count == 1);
    trigger.signal();
    REQUIRE(count == 2);
  }

  TEST_CASE("signalling_without_a_slot") {
    auto trigger = Trigger();
    trigger.signal();
    trigger.signal();
  }

  TEST_CASE("the_trigger_in_use") {
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

  TEST_CASE("the_trigger_in_use_is_per_thread") {
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

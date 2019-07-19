#include <catch2/catch.hpp>
#include "Aspen/Queue.hpp"

using namespace Aspen;

TEST_CASE("test_queue_immediate_complete", "[Queue]") {
  auto trigger = Trigger([] (int) {});
  Trigger::set_trigger(trigger);
  auto queue = Queue<int>();
  queue.commit(0);
  queue.set_complete();
  queue.commit(1);
}

#ifndef ASPEN_TRIGGER_HPP
#define ASPEN_TRIGGER_HPP
#include <functional>
#include <utility>

namespace Aspen {

  /** Used to indicate an asynchronous update available in a reactor. */
  class Trigger {
    public:

      /**
       * Type of callback used when an update is available.
       * @param sequence The update's sequence number.
       */
      using Slot = std::function<void ()>;

      /** Returns the Trigger currently being used. */
      static Trigger& get_trigger();

      /** Sets the Trigger to use within this thread. */
      static void set_trigger(Trigger& trigger);

      /** Constructs a Trigger with an initial sequence of 0. */
      Trigger(Slot slot);

      /**
       * Signals an update is available.
       */
      void signal();

   private:
      static inline thread_local Trigger* m_trigger = nullptr;
      Slot m_slot;

      Trigger(const Trigger&) = delete;
      Trigger& operator =(const Trigger&) = delete;
  };

  inline Trigger& Trigger::get_trigger() {
    return *m_trigger;
  }

  inline void Trigger::set_trigger(Trigger& trigger) {
    m_trigger = &trigger;
  }

  inline Trigger::Trigger(Slot slot)
      : m_slot(std::move(slot)) {}

  inline void Trigger::signal() {
    m_slot();
  }
}

#endif

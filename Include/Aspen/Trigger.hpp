#ifndef ASPEN_TRIGGER_HPP
#define ASPEN_TRIGGER_HPP
#include <atomic>
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
      using Slot = std::function<void (int sequence)>;

      /** Returns the Trigger currently being used. */
      static Trigger& get_trigger();

      /** Sets the Trigger to use within this thread. */
      static void set_trigger(Trigger& trigger);

      /** Constructs a Trigger with an initial sequence of 0. */
      Trigger(Slot slot);

      /**
       * Signals an update is available.
       * @param sequence The update's sequence number.
       */
      void signal(int& sequence);

   private:
      static inline thread_local Trigger* m_trigger = nullptr;
      std::atomic_int m_sequence;
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
      : m_sequence(0),
        m_slot(std::move(slot)) {}

  inline void Trigger::signal(int& sequence) {
    sequence = ++m_sequence;
    m_slot(sequence);
  }
}

#endif

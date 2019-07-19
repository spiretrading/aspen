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

      /** Constructs a Trigger with an initial sequence of 0. */
      Trigger(Slot slot);

      /**
       * Signals an update is available.
       * @param sequence The update's sequence number.
       */
      void signal(int& sequence);

   private:
      std::atomic_int m_sequence;
      Slot m_slot;

      Trigger(const Trigger&) = delete;
      Trigger& operator =(const Trigger&) = delete;
  };

  inline Trigger::Trigger(Slot slot)
      : m_sequence(0),
        m_slot(std::move(slot)) {}

  inline void Trigger::signal(int& sequence) {
    sequence = ++m_sequence;
    m_slot(sequence);
  }
}

#endif

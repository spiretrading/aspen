#ifndef ASPEN_TRIGGER_HPP
#define ASPEN_TRIGGER_HPP
#include <functional>
#include <utility>
#include "Aspen/Python/DllExports.hpp"

namespace Aspen {
  class Trigger;
namespace Details {
  struct ASPEN_EXPORT_DLL StaticTrigger {
    static Trigger*& get() noexcept;
  };

#ifndef ASPEN_USE_DLL
  ASPEN_EMIT_DLL inline Trigger*& StaticTrigger::get() noexcept {
    static thread_local auto current_trigger = static_cast<Trigger*>(nullptr);
    return current_trigger;
  }
#endif
}

  /** Used to indicate an asynchronous update available in a reactor. */
  class Trigger {
    public:

      /**
       * Type of callback used to indicate an update is available.
       */
      using Slot = std::function<void ()>;

      /** Returns the Trigger currently being used. */
      static Trigger* get_trigger();

      /** Sets the Trigger to use within this thread. */
      static void set_trigger(Trigger* trigger);

      /** Sets the Trigger to use within this thread. */
      static void set_trigger(Trigger& trigger);

      /**
       * Constructs a Trigger with no slot.
       */
      Trigger();

      /**
       * Constructs a Trigger.
       * @param slot The function to call when an update is available.
       */
      explicit Trigger(Slot slot);

      /**
       * Signals an update is available.
       */
      void signal();

   private:
      Slot m_slot;

      Trigger(const Trigger&) = delete;
      Trigger(Trigger&&) = delete;
      Trigger& operator =(const Trigger&) = delete;
      Trigger& operator =(Trigger&&) = delete;
  };

  inline Trigger* Trigger::get_trigger() {
    return Details::StaticTrigger::get();
  }

  inline void Trigger::set_trigger(Trigger* trigger) {
    Details::StaticTrigger::get() = trigger;
  }

  inline void Trigger::set_trigger(Trigger& trigger) {
    set_trigger(&trigger);
  }

  inline Trigger::Trigger()
    : Trigger([] {}) {}

  inline Trigger::Trigger(Slot slot)
    : m_slot(std::move(slot)) {}

  inline void Trigger::signal() {
    m_slot();
  }
}

#endif

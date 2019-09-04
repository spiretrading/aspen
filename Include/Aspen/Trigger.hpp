#ifndef ASPEN_TRIGGER_HPP
#define ASPEN_TRIGGER_HPP
#include <functional>
#include <utility>
#include "Aspen/Python/DllExports.hpp"

namespace Aspen {
  class Trigger;
namespace Details {
#if !defined(ASPEN_BUILD_DLL) && !defined(ASPEN_USE_DLL)
  template<typename T>
  struct StaticTrigger {
    static thread_local Trigger* m_value;

    static Trigger*& get() {
      return m_value;
    }
  };

  template<typename T>
  thread_local Trigger* StaticTrigger<T>::m_value;
#elif defined(ASPEN_BUILD_DLL)
 template<typename T>
  struct StaticTrigger {
    ASPEN_EXPORT_DLL static Trigger*& get() {
      static thread_local Trigger* value;
      return value;
    }
  };
#elif defined(ASPEN_USE_DLL)
  template<typename T>
  struct StaticTrigger {
    ASPEN_EXPORT_DLL static Trigger*& get();
  };
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
      Trigger& operator =(const Trigger&) = delete;
  };

  inline Trigger* Trigger::get_trigger() {
    return Details::StaticTrigger<void>::get();
  }

  inline void Trigger::set_trigger(Trigger* trigger) {
    Details::StaticTrigger<void>::get() = trigger;
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

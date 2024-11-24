module;
#include <condition_variable>
#include <mutex>
#if defined WIN32
  #include <windows.h>
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  #include <signal.h>
#endif
#include <set>

export module Aspen:Executor;

import :Box;
import :Trigger;

export namespace Aspen {

  /** Provides a synchronized environment for running a single reactor. */
  class Executor {
    public:

      /**
       * Constructs an Executor.
       * @param reactor The reactor to execute.
       */
      template<typename R>
      explicit Executor(R&& reactor);

      /** Repeatedly executes the reactor until it completes or evaluates to
       *  NONE.
       */
      void run_until_none();

      /** Repeatedly executes the reactor until it completes. */
      void run_until_complete();

    private:
      enum class Update : char {
        NONE,
        UPDATE,
        ABORT
      };
      static inline std::mutex m_abort_mutex;
      static inline std::set<Executor*> m_running_executors;
      std::mutex m_mutex;
      std::condition_variable m_update_condition;
      Trigger m_trigger;
      int m_sequence;
      Box<void> m_reactor;
      Update m_has_update;

      void abort();
      void on_update();
#if defined WIN32
      static BOOL __stdcall ctrl_handler(DWORD ctrl);
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
      static void ctrl_handler(int sig);
#endif
      Executor(const Executor&) = delete;
      Executor& operator =(const Executor&) = delete;
  };

  template<typename R>
  Executor::Executor(R&& reactor)
    : m_trigger([this] { on_update(); }),
      m_sequence(0),
      m_reactor(std::forward<R>(reactor)),
      m_has_update(Update::NONE) {}

  inline void Executor::run_until_none() {
    auto old_trigger = Trigger::get_trigger();
    Trigger::set_trigger(m_trigger);
    while(has_continuation(m_reactor.commit(m_sequence))) {
      ++m_sequence;
    }
    ++m_sequence;
    Trigger::set_trigger(old_trigger);
  }

  inline void Executor::run_until_complete() {
    auto old_trigger = Trigger::get_trigger();
    Trigger::set_trigger(m_trigger);
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    auto previous_handler = static_cast<sighandler_t>(nullptr);
#endif
    {
      auto lock = std::lock_guard(m_abort_mutex);
#if defined WIN32
      ::SetConsoleCtrlHandler(reinterpret_cast<PHANDLER_ROUTINE>(&ctrl_handler),
        TRUE);
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
      previous_handler = ::signal(SIGINT, &ctrl_handler);
#endif
      m_running_executors.insert(this);
    }
    while(true) {
      auto state = m_reactor.commit(m_sequence);
      ++m_sequence;
      if(is_complete(state)) {
        break;
      } else if(!has_continuation(state)) {
        auto lock = std::unique_lock(m_mutex);
        while(m_has_update == Update::NONE) {
          m_update_condition.wait(lock);
        }
        if(m_has_update == Update::ABORT) {
          break;
        }
        m_has_update = Update::NONE;
      }
    }
    {
      auto lock = std::lock_guard(m_abort_mutex);
      m_running_executors.erase(this);
#if defined WIN32
    ::SetConsoleCtrlHandler(reinterpret_cast<PHANDLER_ROUTINE>(&ctrl_handler),
      FALSE);
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
    ::signal(SIGINT, previous_handler);
#endif
    }
    Trigger::set_trigger(old_trigger);
  }

  inline void Executor::abort() {
    {
      auto lock = std::lock_guard(m_mutex);
      m_has_update = Update::ABORT;
    }
    m_update_condition.notify_one();
  }

  inline void Executor::on_update() {
    {
      auto lock = std::lock_guard(m_mutex);
      m_has_update = Update::UPDATE;
    }
    m_update_condition.notify_one();
  }

#if defined WIN32
  inline BOOL __stdcall Executor::ctrl_handler(DWORD ctrl) {
    switch(ctrl) {
      case CTRL_C_EVENT:
        {
          auto lock = std::lock_guard(m_abort_mutex);
          for(auto& executor : m_running_executors) {
            executor->abort();
          }
        }
        return true;
    }
    return false;
  }
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  inline void Executor::ctrl_handler(int sig) {
    auto lock = std::lock_guard(m_abort_mutex);
    for(auto& executor : m_running_executors) {
      executor->abort();
    }
  }
#endif
}

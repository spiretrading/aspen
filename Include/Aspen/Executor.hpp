#ifndef ASPEN_EXECUTOR_HPP
#define ASPEN_EXECUTOR_HPP
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>
#if defined(_WIN32)
  #include <windows.h>
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  #include <signal.h>
#endif
#include "Aspen/Box.hpp"
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Trigger.hpp"

namespace Aspen {

  /** Provides a synchronized environment for running a single reactor. */
  class Executor {
    public:

      /**
       * Constructs an Executor.
       * @param reactor The reactor to execute.
       */
      template<typename R> requires IsReactor<std::remove_cvref_t<R>>
      explicit Executor(R&& reactor);

      /**
       * Repeatedly executes the reactor until it completes or no longer
       * requires an immediate commit.
       */
      void run_until_none();

      /** Repeatedly executes the reactor until it completes. */
      void run_until_complete();

      /** Stops a run in progress, callable from any thread. */
      void abort();

    private:
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
      using SignalAction = struct ::sigaction;
#endif
      struct RunningScope {
        Executor* m_executor;
        Trigger* m_previous;

        explicit RunningScope(Executor& executor);
        ~RunningScope();

        RunningScope(const RunningScope&) = delete;
        RunningScope& operator =(const RunningScope&) = delete;
      };
      static constexpr auto INTERRUPT_INTERVAL = std::chrono::seconds(1);
      static inline std::mutex m_abort_mutex;
      static inline std::vector<Executor*> m_running_executors;
      static inline std::atomic_bool m_is_interrupted;
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
      static inline auto m_previous_action = SignalAction();
#endif
      std::mutex m_mutex;
      std::condition_variable m_update_condition;
      Trigger m_trigger;
      CommitFlag m_flag;
      std::uint64_t m_sequence;
      Box<void> m_reactor;
      std::atomic_bool m_is_aborted;
      bool m_is_complete;
      bool m_has_update;

      void on_update();
      bool is_aborted() const noexcept;
      State commit();
#if defined(_WIN32)
      static BOOL __stdcall ctrl_handler(DWORD ctrl);
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
      static void ctrl_handler(int);
#endif
      Executor(const Executor&) = delete;
      Executor& operator =(const Executor&) = delete;
  };

  template<typename R> requires IsReactor<std::remove_cvref_t<R>>
  Executor::Executor(R&& reactor)
      : m_trigger([this] { on_update(); }),
        m_sequence(0),
        m_reactor(std::forward<R>(reactor)),
        m_is_aborted(false),
        m_is_complete(false),
        m_has_update(false) {
    m_flag.set_trigger(&m_trigger);
  }

  inline bool Executor::is_aborted() const noexcept {
    return m_is_aborted.load(std::memory_order_acquire) ||
      m_is_interrupted.load(std::memory_order_acquire);
  }

  inline State Executor::commit() {
    m_flag.clear();
    auto scope = CommitFlagScope(m_flag);
    auto state = m_reactor.commit(m_sequence);
    ++m_sequence;
    if(is_complete(state)) {
      m_is_complete = true;
    }
    return state;
  }

  inline void Executor::run_until_none() {
    if(m_is_complete) {
      return;
    }
    auto old_trigger = Trigger::get_trigger();
    Trigger::set_trigger(m_trigger);
    while(!m_is_aborted.load(std::memory_order_acquire) && !m_is_complete &&
      has_continuation(commit())) {}
    Trigger::set_trigger(old_trigger);
  }

  inline void Executor::run_until_complete() {
    if(m_is_complete) {
      return;
    }
    auto scope = RunningScope(*this);
    while(!is_aborted()) {
      auto state = commit();
      if(is_complete(state)) {
        break;
      }
      if(!has_continuation(state)) {
        auto lock = std::unique_lock(m_mutex);
        while(!m_update_condition.wait_for(lock, INTERRUPT_INTERVAL, [&] {
          return m_has_update || is_aborted();
        })) {}
        m_has_update = false;
      }
    }
  }

  inline Executor::RunningScope::RunningScope(Executor& executor)
      : m_executor(&executor),
        m_previous(Trigger::get_trigger()) {
    {
      auto lock = std::lock_guard(m_abort_mutex);
      m_running_executors.push_back(&executor);
      if(m_running_executors.size() == 1) {
        m_is_interrupted.store(false, std::memory_order_release);
#if defined(_WIN32)
        ::SetConsoleCtrlHandler(&ctrl_handler, TRUE);
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
        auto action = SignalAction();
        action.sa_handler = &ctrl_handler;
        ::sigemptyset(&action.sa_mask);
        action.sa_flags = SA_RESTART;
        ::sigaction(SIGINT, &action, &m_previous_action);
#endif
      }
    }
    Trigger::set_trigger(executor.m_trigger);
  }

  inline Executor::RunningScope::~RunningScope() {
    Trigger::set_trigger(m_previous);
    auto lock = std::lock_guard(m_abort_mutex);
    std::erase(m_running_executors, m_executor);
    if(m_running_executors.empty()) {
#if defined(_WIN32)
      ::SetConsoleCtrlHandler(&ctrl_handler, FALSE);
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
      ::sigaction(SIGINT, &m_previous_action, nullptr);
#endif
      m_is_interrupted.store(false, std::memory_order_release);
    }
  }

  inline void Executor::abort() {
    {
      auto lock = std::lock_guard(m_mutex);
      m_is_aborted.store(true, std::memory_order_release);
    }
    m_update_condition.notify_one();
  }

  inline void Executor::on_update() {
    {
      auto lock = std::lock_guard(m_mutex);
      m_has_update = true;
    }
    m_update_condition.notify_one();
  }

#if defined(_WIN32)
  inline BOOL __stdcall Executor::ctrl_handler(DWORD ctrl) {
    if(ctrl != CTRL_C_EVENT) {
      return FALSE;
    }
    m_is_interrupted.store(true, std::memory_order_release);
    return TRUE;
  }
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
  inline void Executor::ctrl_handler(int) {
    m_is_interrupted.store(true, std::memory_order_release);
  }
#endif
}

#endif

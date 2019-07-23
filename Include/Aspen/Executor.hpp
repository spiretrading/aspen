#ifndef ASPEN_EXECUTOR_HPP
#define ASPEN_EXECUTOR_HPP
#include <condition_variable>
#include <mutex>
#include "Aspen/Box.hpp"
#include "Aspen/Trigger.hpp"

namespace Aspen {

  /** Provides a synchronized environment for running a single reactor. */
  class Executor {
    public:

      /**
       * Constructs an Executor.
       * @param reactor The reactor to execute.
       */
      explicit Executor(Box<void> reactor);

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
      std::mutex m_mutex;
      std::condition_variable m_update_condition;
      Trigger m_trigger;
      int m_sequence;
      Box<void> m_reactor;
      bool m_has_update;

      void on_update();
  };

  inline Executor::Executor(Box<void> reactor)
    : m_trigger([=] { on_update(); }),
      m_sequence(0),
      m_reactor(std::move(reactor)),
      m_has_update(false) {}

  template<typename R>
  Executor::Executor(R&& reactor)
    : Executor(Box(std::forward<R>(reactor))) {}

  inline void Executor::run_until_none() {
    auto old_trigger = Trigger::get_trigger();
    Trigger::set_trigger(m_trigger);
    while(true) {
      auto state = m_reactor.commit(m_sequence);
      ++m_sequence;
      if(state == State::NONE || is_complete(state)) {
        break;
      }
    }
    Trigger::set_trigger(old_trigger);
  }

  inline void Executor::run_until_complete() {
    auto old_trigger = Trigger::get_trigger();
    Trigger::set_trigger(m_trigger);
    while(true) {
      auto state = m_reactor.commit(m_sequence);
      ++m_sequence;
      if(is_complete(state)) {
        break;
      } else if(state == State::NONE) {
        auto lock = std::unique_lock(m_mutex);
        while(!m_has_update) {
          m_update_condition.wait(lock);
        }
        m_has_update = false;
      }
    }
    Trigger::set_trigger(old_trigger);
  }

  inline void Executor::on_update() {
    {
      auto lock = std::lock_guard(m_mutex);
      m_has_update = true;
    }
    m_update_condition.notify_one();
  }
}

#endif

#ifndef ASPEN_COMMIT_FLAG_HPP
#define ASPEN_COMMIT_FLAG_HPP
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <vector>
#include "Aspen/Python/DllExports.hpp"
#include "Aspen/Trigger.hpp"

namespace Aspen {
  class CommitFlag;

namespace Details {
  struct ASPEN_EXPORT_DLL StaticCommitFlag {
    static CommitFlag*& get() noexcept;
  };

#ifndef ASPEN_USE_DLL
  ASPEN_EMIT_DLL inline CommitFlag*& StaticCommitFlag::get() noexcept {
    static thread_local auto current_flag = static_cast<CommitFlag*>(nullptr);
    return current_flag;
  }
#endif
}

  /**
   * Records whether a reactor requires a commit and propagates that requirement
   * to the reactors depending on it.
   */
  class CommitFlag {
    public:

      /** Returns the CommitFlag that the reactor being committed reports to. */
      static CommitFlag* get_current() noexcept;

      /** Constructs a raised CommitFlag with no dependents. */
      CommitFlag() noexcept;

      ~CommitFlag();

      /** Returns <code>true</code> iff a commit is required. */
      bool is_raised() const noexcept;

      /** Returns <code>true</code> iff this flag was assigned a slot. */
      bool has_slot() const noexcept;

      /** Indicates that a commit is required. */
      void raise() noexcept;

      /** Indicates that a commit is no longer required. */
      void clear() noexcept;

      /**
       * Sets the sole CommitFlag to raise whenever this CommitFlag is raised.
       * @param parent The CommitFlag to raise, or <code>nullptr</code> for
       *        none.
       */
      void set_parent(CommitFlag* parent) noexcept;

      /**
       * Adds a CommitFlag to raise whenever this CommitFlag is raised.
       * @param parent The CommitFlag to raise.
       */
      void add_parent(CommitFlag& parent);

      /**
       * Removes a CommitFlag previously added as a dependent.
       * @param parent The CommitFlag to stop raising.
       */
      void remove_parent(CommitFlag& parent) noexcept;

      /**
       * Sets a bit to raise whenever this CommitFlag is raised, letting a
       * dependent locate its raised children without scanning all of them.
       * @param word The word containing the bit to raise.
       * @param bit The index of the bit to raise.
       */
      void set_slot(std::atomic_uint64_t* word, std::uint8_t bit) noexcept;

      /**
       * Sets the Trigger to signal when this CommitFlag is raised, marking it
       * as the root of a graph.
       * @param trigger The Trigger to signal.
       */
      void set_trigger(Trigger* trigger) noexcept;

    private:
      static constexpr auto RAISED = std::uint8_t(1);
      static constexpr auto PROPAGATED = std::uint8_t(2);
      enum class Kind : std::uint8_t {
        PLAIN,
        HUB,
        ROOT
      };
      union {
        CommitFlag* m_parent;
        Trigger* m_trigger;
      };
      union {
        std::vector<CommitFlag*>* m_parents;
        std::atomic_uint64_t* m_word;
      };
      std::atomic_uint8_t m_flags;
      std::uint8_t m_bit;
      Kind m_kind;

      CommitFlag(const CommitFlag&) = delete;
      CommitFlag(CommitFlag&&) = delete;
      CommitFlag& operator =(const CommitFlag&) = delete;
      CommitFlag& operator =(CommitFlag&&) = delete;
  };

  /** Sets the CommitFlag that the reactor being committed reports to. */
  class CommitFlagScope {
    public:

      /**
       * Constructs a CommitFlagScope.
       * @param flag The CommitFlag to report to for the duration of this scope.
       */
      explicit CommitFlagScope(CommitFlag& flag) noexcept;

      ~CommitFlagScope();

    private:
      CommitFlag* m_previous;

      CommitFlagScope(const CommitFlagScope&) = delete;
      CommitFlagScope& operator =(const CommitFlagScope&) = delete;
  };

  inline CommitFlag* CommitFlag::get_current() noexcept {
    return Details::StaticCommitFlag::get();
  }

  inline CommitFlag::CommitFlag() noexcept
    : m_parent(nullptr),
      m_word(nullptr),
      m_flags(RAISED),
      m_bit(0),
      m_kind(Kind::PLAIN) {}

  inline CommitFlag::~CommitFlag() {
    if(m_kind == Kind::HUB) {
      delete m_parents;
    }
  }

  inline bool CommitFlag::is_raised() const noexcept {
    return (m_flags.load(std::memory_order_acquire) & RAISED) != 0;
  }

  inline bool CommitFlag::has_slot() const noexcept {
    return m_kind != Kind::HUB && m_word;
  }

  inline void CommitFlag::raise() noexcept {
    if(m_kind == Kind::HUB) {
      auto previous =
        m_flags.fetch_or(RAISED | PROPAGATED, std::memory_order_acq_rel);
      if((previous & PROPAGATED) != 0) {
        return;
      }
      if(m_parent) {
        m_parent->raise();
      }
      if(m_parents) {
        for(auto parent : *m_parents) {
          parent->raise();
        }
      }
    } else {
      auto previous = m_flags.fetch_or(RAISED, std::memory_order_acq_rel);
      if((previous & RAISED) != 0) {
        return;
      }
      if(m_word) {
        m_word->fetch_or(std::uint64_t(1) << m_bit, std::memory_order_release);
      }
      if(m_kind == Kind::ROOT) {
        if(m_trigger) {
          m_trigger->signal();
        }
      } else if(m_parent) {
        m_parent->raise();
      }
    }
  }

  inline void CommitFlag::clear() noexcept {
    m_flags.exchange(0, std::memory_order_acq_rel);
  }

  inline void CommitFlag::set_parent(CommitFlag* parent) noexcept {
    m_parent = parent;
  }

  inline void CommitFlag::set_trigger(Trigger* trigger) noexcept {
    m_trigger = trigger;
    m_kind = Kind::ROOT;
  }

  inline void CommitFlag::set_slot(
      std::atomic_uint64_t* word, std::uint8_t bit) noexcept {
    m_word = word;
    m_bit = bit;
    if(m_word && is_raised()) {
      m_word->fetch_or(std::uint64_t(1) << m_bit, std::memory_order_release);
    }
  }

  inline void CommitFlag::add_parent(CommitFlag& parent) {
    if(m_kind != Kind::HUB) {
      m_kind = Kind::HUB;
      m_parents = nullptr;
    }
    if(!m_parent) {
      m_parent = &parent;
    } else {
      if(!m_parents) {
        m_parents = new std::vector<CommitFlag*>();
      }
      m_parents->push_back(&parent);
    }
    if(is_raised()) {
      parent.raise();
    }
  }

  inline void CommitFlag::remove_parent(CommitFlag& parent) noexcept {
    if(m_parent == &parent) {
      if(m_parents && !m_parents->empty()) {
        m_parent = m_parents->back();
        m_parents->pop_back();
      } else {
        m_parent = nullptr;
      }
      return;
    }
    if(!m_parents) {
      return;
    }
    auto i = std::find(m_parents->begin(), m_parents->end(), &parent);
    if(i != m_parents->end()) {
      m_parents->erase(i);
    }
  }

  inline CommitFlagScope::CommitFlagScope(CommitFlag& flag) noexcept
      : m_previous(CommitFlag::get_current()) {
    Details::StaticCommitFlag::get() = &flag;
  }

  inline CommitFlagScope::~CommitFlagScope() {
    Details::StaticCommitFlag::get() = m_previous;
  }
}

#endif

#ifndef ASPEN_COMMIT_FLAG_HPP
#define ASPEN_COMMIT_FLAG_HPP
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
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
       * Removes a CommitFlag previously added as a dependent, returning once
       * no propagation still in progress can reach it.
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
      using Parents = std::vector<CommitFlag*>;
      std::atomic<void*> m_pointer;
      std::atomic<std::atomic_uint64_t*> m_word;
      std::atomic<std::shared_ptr<const Parents>> m_parents;
      std::atomic_uint32_t m_readers;
      std::atomic_uint8_t m_flags;
      std::atomic<std::uint8_t> m_bit;
      std::atomic<Kind> m_kind;

      CommitFlag(const CommitFlag&) = delete;
      CommitFlag(CommitFlag&&) = delete;
      CommitFlag& operator =(const CommitFlag&) = delete;
      CommitFlag& operator =(CommitFlag&&) = delete;
      void propagate() noexcept;
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
    : m_pointer(nullptr),
      m_word(nullptr),
      m_readers(0),
      m_flags(RAISED),
      m_bit(0),
      m_kind(Kind::PLAIN) {}

  inline bool CommitFlag::is_raised() const noexcept {
    return (m_flags.load(std::memory_order_acquire) & RAISED) != 0;
  }

  inline bool CommitFlag::has_slot() const noexcept {
    return m_kind.load(std::memory_order_acquire) != Kind::HUB &&
      m_word.load(std::memory_order_acquire);
  }

  inline void CommitFlag::raise() noexcept {
    m_readers.fetch_add(1);
    propagate();
    m_readers.fetch_sub(1);
    m_readers.notify_all();
  }

  inline void CommitFlag::propagate() noexcept {
    auto kind = m_kind.load(std::memory_order_acquire);
    if(kind == Kind::HUB) {
      auto previous =
        m_flags.fetch_or(RAISED | PROPAGATED, std::memory_order_acq_rel);
      if((previous & PROPAGATED) != 0) {
        return;
      }
      if(auto parent = static_cast<CommitFlag*>(
          m_pointer.load(std::memory_order_acquire))) {
        parent->raise();
      }
      if(auto parents = m_parents.load(std::memory_order_acquire)) {
        for(auto parent : *parents) {
          parent->raise();
        }
      }
    } else {
      auto previous = m_flags.fetch_or(RAISED, std::memory_order_acq_rel);
      if((previous & RAISED) != 0) {
        return;
      }
      if(auto word = m_word.load(std::memory_order_acquire)) {
        word->fetch_or(
          std::uint64_t(1) << m_bit.load(std::memory_order_acquire),
          std::memory_order_release);
      }
      if(kind == Kind::ROOT) {
        if(auto trigger = static_cast<Trigger*>(
            m_pointer.load(std::memory_order_acquire))) {
          trigger->signal();
        }
      } else if(auto parent = static_cast<CommitFlag*>(
          m_pointer.load(std::memory_order_acquire))) {
        parent->raise();
      }
    }
  }

  inline void CommitFlag::clear() noexcept {
    m_flags.exchange(0, std::memory_order_acq_rel);
  }

  inline void CommitFlag::set_parent(CommitFlag* parent) noexcept {
    assert(m_kind.load(std::memory_order_relaxed) != Kind::ROOT);
    m_pointer.store(parent, std::memory_order_release);
  }

  inline void CommitFlag::set_trigger(Trigger* trigger) noexcept {
    assert(m_kind.load(std::memory_order_relaxed) == Kind::PLAIN &&
      !m_pointer.load(std::memory_order_relaxed));
    m_pointer.store(trigger, std::memory_order_release);
    m_kind.store(Kind::ROOT, std::memory_order_release);
  }

  inline void CommitFlag::set_slot(
      std::atomic_uint64_t* word, std::uint8_t bit) noexcept {
    assert(m_kind.load(std::memory_order_relaxed) != Kind::HUB);
    m_bit.store(bit, std::memory_order_release);
    m_word.store(word, std::memory_order_release);
    if(word && is_raised()) {
      word->fetch_or(std::uint64_t(1) << bit, std::memory_order_release);
    }
  }

  inline void CommitFlag::add_parent(CommitFlag& parent) {
    assert(!has_slot());
    m_kind.store(Kind::HUB, std::memory_order_release);
    if(!m_pointer.load(std::memory_order_relaxed)) {
      m_pointer.store(&parent, std::memory_order_release);
    } else {
      auto parents = m_parents.load(std::memory_order_relaxed);
      auto updated = [&] {
        if(parents) {
          return std::make_shared<Parents>(*parents);
        }
        return std::make_shared<Parents>();
      }();
      updated->push_back(&parent);
      m_parents.store(std::move(updated), std::memory_order_release);
    }
    if(is_raised()) {
      parent.raise();
    }
  }

  inline void CommitFlag::remove_parent(CommitFlag& parent) noexcept {
    auto parents = m_parents.load(std::memory_order_relaxed);
    if(m_pointer.load(std::memory_order_relaxed) == &parent) {
      if(parents && !parents->empty()) {
        auto updated = std::make_shared<Parents>(*parents);
        m_pointer.store(updated->back(), std::memory_order_release);
        updated->pop_back();
        m_parents.store(std::move(updated), std::memory_order_release);
      } else {
        m_pointer.store(nullptr, std::memory_order_release);
      }
    } else if(parents) {
      auto i = std::find(parents->begin(), parents->end(), &parent);
      if(i != parents->end()) {
        auto updated = std::make_shared<Parents>(*parents);
        updated->erase(updated->begin() + (i - parents->begin()));
        m_parents.store(std::move(updated), std::memory_order_release);
      }
    }
    auto readers = m_readers.load();
    while(readers != 0) {
      m_readers.wait(readers);
      readers = m_readers.load();
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

#ifndef ASPEN_BRANCH_HPP
#define ASPEN_BRANCH_HPP
#include <atomic>
#include <concepts>
#include <cstdint>
#include <utility>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"

namespace Aspen {

  /**
   * Pairs a reactor with the CommitFlag used to determine whether it requires
   * a commit, allowing a reactor that manages its children directly to skip
   * those whose subtree has no pending update.
   * @param <R> The type of reactor to manage.
   */
  template<IsReactor R>
  class Branch {
    public:

      /** The type of reactor being managed. */
      using Reactor = R;

      /** Constructs an unlinked Branch. */
      Branch();

      /**
       * Constructs a Branch.
       * @param reactor The reactor to manage.
       */
      template<typename A> requires std::constructible_from<R, A>
      explicit Branch(A&& reactor);

      Branch(const Branch& branch);
      Branch(Branch&& branch) noexcept;

      /**
       * Sets a bit to raise whenever this Branch requires a commit.
       * @param word The word containing the bit to raise.
       * @param bit The index of the bit to raise.
       */
      void set_slot(std::atomic_uint64_t* word, std::uint8_t bit) noexcept;

      auto& operator *(this auto&& self) noexcept;
      auto* operator ->(this auto&& self) noexcept;
      State commit(std::uint64_t sequence) noexcept;
      Branch& operator =(const Branch& branch);
      Branch& operator =(Branch&& branch) noexcept;

    private:
      CommitFlag m_flag;
      [[no_unique_address]]
      Reactor m_reactor;
      State m_state;
      bool m_is_linked;
  };

  template<typename R>
  Branch(R&&) -> Branch<std::decay_t<R>>;

  template<IsReactor R>
  Branch<R>::Branch()
    : m_state(State::NONE),
      m_is_linked(false) {}

  template<IsReactor R>
  template<typename A> requires std::constructible_from<R, A>
  Branch<R>::Branch(A&& reactor)
    : m_reactor(std::forward<A>(reactor)),
      m_state(State::NONE),
      m_is_linked(false) {}

  template<IsReactor R>
  Branch<R>::Branch(const Branch& branch)
    : m_reactor(branch.m_reactor),
      m_state(branch.m_state),
      m_is_linked(false) {}

  template<IsReactor R>
  Branch<R>::Branch(Branch&& branch) noexcept
    : m_reactor(std::move(branch.m_reactor)),
      m_state(branch.m_state),
      m_is_linked(false) {}

  template<IsReactor R>
  void Branch<R>::set_slot(
      std::atomic_uint64_t* word, std::uint8_t bit) noexcept {
    m_flag.set_slot(word, bit);
  }

  template<IsReactor R>
  auto& Branch<R>::operator *(this auto&& self) noexcept {
    return self.m_reactor;
  }

  template<IsReactor R>
  auto* Branch<R>::operator ->(this auto&& self) noexcept {
    return &self.m_reactor;
  }

  template<IsReactor R>
  State Branch<R>::commit(std::uint64_t sequence) noexcept {
    if(!m_is_linked) {
      m_is_linked = true;
      m_flag.set_parent(CommitFlag::get_current());
    }
    if(!m_flag.is_raised()) {
      return reset(m_state, combine(State::EVALUATED, State::CONTINUE));
    }
    m_flag.clear();
    m_state = [&] {
      auto scope = CommitFlagScope(m_flag);
      return m_reactor.commit(sequence);
    }();
    if(has_continuation(m_state)) {
      m_flag.raise();
    }
    return m_state;
  }

  template<IsReactor R>
  Branch<R>& Branch<R>::operator =(const Branch& branch) {
    m_reactor = branch.m_reactor;
    m_state = branch.m_state;
    m_flag.raise();
    return *this;
  }

  template<IsReactor R>
  Branch<R>& Branch<R>::operator =(Branch&& branch) noexcept {
    m_reactor = std::move(branch.m_reactor);
    m_state = branch.m_state;
    m_flag.raise();
    return *this;
  }
}

#endif

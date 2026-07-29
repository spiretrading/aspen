#ifndef ASPEN_BRANCH_HPP
#define ASPEN_BRANCH_HPP
#include <cstdint>
#include <type_traits>
#include <utility>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/State.hpp"

namespace Aspen {

  /**
   * Pairs a reactor with the CommitFlag used to determine whether it requires
   * a commit, allowing a reactor that manages its children directly to skip
   * those whose subtree has no pending update.
   * @param <R> The type of reactor to manage.
   */
  template<typename R>
  class Branch {
    public:

      /** The type of reactor being managed. */
      using Reactor = R;

      /** Constructs a Branch. */
      Branch();

      /**
       * Constructs a Branch.
       * @param reactor The reactor to manage.
       */
      template<typename A, typename = std::enable_if_t<
        !std::is_base_of_v<Branch, std::decay_t<A>>>>
      explicit Branch(A&& reactor);

      /** Copies a Branch. */
      Branch(const Branch& branch);

      /** Moves a Branch. */
      Branch(Branch&& branch) noexcept;

      /** Returns a reference to the reactor. */
      const Reactor& operator *() const noexcept;

      /** Returns a pointer to the reactor. */
      const Reactor* operator ->() const noexcept;

      /** Returns a reference to the reactor. */
      Reactor& operator *() noexcept;

      /** Returns a pointer to the reactor. */
      Reactor* operator ->() noexcept;

      State commit(std::uint64_t sequence) noexcept;

      /** Copies a Branch. */
      Branch& operator =(const Branch& branch);

      /** Moves a Branch. */
      Branch& operator =(Branch&& branch) noexcept;

    private:
      CommitFlag m_flag;
      Reactor m_reactor;
      State m_state;
      bool m_is_linked;
  };

  template<typename R>
  Branch(R&&) -> Branch<std::decay_t<R>>;

  template<typename R>
  Branch<R>::Branch()
    : m_state(State::NONE),
      m_is_linked(false) {}

  template<typename R>
  template<typename A, typename>
  Branch<R>::Branch(A&& reactor)
    : m_reactor(std::forward<A>(reactor)),
      m_state(State::NONE),
      m_is_linked(false) {}

  template<typename R>
  Branch<R>::Branch(const Branch& branch)
    : m_reactor(branch.m_reactor),
      m_state(branch.m_state),
      m_is_linked(false) {}

  template<typename R>
  Branch<R>::Branch(Branch&& branch) noexcept
    : m_reactor(std::move(branch.m_reactor)),
      m_state(branch.m_state),
      m_is_linked(false) {}

  template<typename R>
  const typename Branch<R>::Reactor& Branch<R>::operator *() const noexcept {
    return m_reactor;
  }

  template<typename R>
  const typename Branch<R>::Reactor* Branch<R>::operator ->() const noexcept {
    return &m_reactor;
  }

  template<typename R>
  typename Branch<R>::Reactor& Branch<R>::operator *() noexcept {
    return m_reactor;
  }

  template<typename R>
  typename Branch<R>::Reactor* Branch<R>::operator ->() noexcept {
    return &m_reactor;
  }

  template<typename R>
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

  template<typename R>
  Branch<R>& Branch<R>::operator =(const Branch& branch) {
    m_reactor = branch.m_reactor;
    m_state = branch.m_state;
    m_flag.raise();
    return *this;
  }

  template<typename R>
  Branch<R>& Branch<R>::operator =(Branch&& branch) noexcept {
    m_reactor = std::move(branch.m_reactor);
    m_state = branch.m_state;
    m_flag.raise();
    return *this;
  }
}

#endif

#ifndef ASPEN_COMMIT_HANDLER_HPP
#define ASPEN_COMMIT_HANDLER_HPP
#include <atomic>
#include <bit>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/State.hpp"

namespace Aspen {

  /** Helper class used to commit a list of reactors and evaluate to their
   *  aggregate state.
   *  @param <R> The type of reactor to manage.
   */
  template<typename R>
  class CommitHandler {
    public:

      /**
       * Constructs a CommitHandler.
       * @param children The reactors whose commits are to be managed.
       */
      template<typename A = std::allocator<R>>
      explicit CommitHandler(std::vector<R, A> children);

      /** Moves a CommitHandler. */
      CommitHandler(CommitHandler&& handler) noexcept;

      /** Moves a CommitHandler. */
      CommitHandler& operator =(CommitHandler&& handler) noexcept;

      /**
       * Commits all children and returns their aggregate State.
       * @param sequence The commit's sequence.
       * @return The aggregate State of all children.
       */
      State commit(int sequence) noexcept;

      /** Returns the number of reactors managed. */
      std::size_t size() const noexcept;

      /** Returns the reactor at the specified index. */
      const R& get(std::size_t i) const noexcept;

      /** Returns the reactor at the specified index. */
      R& get(std::size_t i) noexcept;

      /**
       * Returns the indices of the reactors that evaluated during the most
       * recent commit.
       */
      const std::vector<std::size_t>& get_evaluated() const noexcept;

    private:
      static constexpr auto BITS = std::size_t(64);
      struct Child {
        CommitFlag m_flag;
        R m_reactor;
        State m_state;
        bool m_has_evaluation;

        Child(R reactor);
        Child(Child&& child) noexcept;
      };
      std::vector<Child> m_children;
      std::vector<std::size_t> m_evaluated;
      std::unique_ptr<std::atomic_uint64_t[]> m_raised;
      std::size_t m_word_count;
      std::size_t m_completion_count;
      std::size_t m_evaluation_count;
      bool m_is_initializing;
      bool m_is_linked;

      void link() noexcept;
  };

  template<typename R>
  CommitHandler<R>::Child::Child(R reactor)
    : m_reactor(std::move(reactor)),
      m_state(State::NONE),
      m_has_evaluation(false) {}

  template<typename R>
  CommitHandler<R>::Child::Child(Child&& child) noexcept
    : m_reactor(std::move(child.m_reactor)),
      m_state(child.m_state),
      m_has_evaluation(child.m_has_evaluation) {}

  template<typename R>
  template<typename A>
  CommitHandler<R>::CommitHandler(std::vector<R, A> children)
      : m_word_count((children.size() + BITS - 1) / BITS),
        m_completion_count(0),
        m_evaluation_count(0),
        m_is_initializing(true),
        m_is_linked(false) {
    m_children.reserve(children.size());
    for(auto& child : children) {
      m_children.push_back(std::move(child));
    }
    m_raised = std::make_unique<std::atomic_uint64_t[]>(m_word_count);
  }

  template<typename R>
  CommitHandler<R>::CommitHandler(CommitHandler&& handler) noexcept
    : m_children(std::move(handler.m_children)),
      m_evaluated(std::move(handler.m_evaluated)),
      m_raised(std::move(handler.m_raised)),
      m_word_count(handler.m_word_count),
      m_completion_count(handler.m_completion_count),
      m_evaluation_count(handler.m_evaluation_count),
      m_is_initializing(handler.m_is_initializing),
      m_is_linked(false) {}

  template<typename R>
  CommitHandler<R>& CommitHandler<R>::operator =(
      CommitHandler&& handler) noexcept {
    m_children = std::move(handler.m_children);
    m_evaluated = std::move(handler.m_evaluated);
    m_raised = std::move(handler.m_raised);
    m_word_count = handler.m_word_count;
    m_completion_count = handler.m_completion_count;
    m_evaluation_count = handler.m_evaluation_count;
    m_is_initializing = handler.m_is_initializing;
    m_is_linked = false;
    return *this;
  }

  template<typename R>
  void CommitHandler<R>::link() noexcept {
    m_is_linked = true;
    auto parent = CommitFlag::get_current();
    for(auto i = std::size_t(0); i != m_children.size(); ++i) {
      auto& flag = m_children[i].m_flag;
      flag.set_parent(parent);
      flag.set_slot(&m_raised[i / BITS], static_cast<std::uint8_t>(i % BITS));
    }
  }

  template<typename R>
  State CommitHandler<R>::commit(int sequence) noexcept {
    if(m_children.empty()) {
      return State::COMPLETE;
    }
    if(!m_is_linked) {
      link();
    }
    m_evaluated.clear();
    auto has_continue = false;
    for(auto word = std::size_t(0); word != m_word_count; ++word) {
      if(m_raised[word].load(std::memory_order_acquire) == 0) {
        continue;
      }
      auto bits = m_raised[word].exchange(0, std::memory_order_acq_rel);
      while(bits != 0) {
        auto index = word * BITS + std::countr_zero(bits);
        bits &= bits - 1;
        auto& child = m_children[index];
        if(is_complete(child.m_state) || !child.m_flag.is_raised()) {
          continue;
        }
        child.m_flag.clear();
        {
          auto scope = CommitFlagScope(child.m_flag);
          child.m_state = child.m_reactor.commit(sequence);
        }
        if(has_evaluation(child.m_state)) {
          m_evaluated.push_back(index);
          if(!child.m_has_evaluation) {
            child.m_has_evaluation = true;
            ++m_evaluation_count;
          }
        }
        if(is_complete(child.m_state)) {
          ++m_completion_count;
          if(!child.m_has_evaluation) {
            return State::COMPLETE;
          }
        } else if(has_continuation(child.m_state)) {
          has_continue = true;
          child.m_flag.raise();
        }
      }
    }
    auto state = State::NONE;
    if(m_is_initializing) {
      if(m_evaluation_count == m_children.size()) {
        m_is_initializing = false;
        state = combine(state, State::EVALUATED);
      }
    } else if(!m_evaluated.empty()) {
      state = combine(state, State::EVALUATED);
    }
    if(m_completion_count == m_children.size()) {
      state = combine(state, State::COMPLETE);
    } else if(has_continue) {
      state = combine(state, State::CONTINUE);
    }
    return state;
  }

  template<typename R>
  const std::vector<std::size_t>&
      CommitHandler<R>::get_evaluated() const noexcept {
    return m_evaluated;
  }

  template<typename R>
  std::size_t CommitHandler<R>::size() const noexcept {
    return m_children.size();
  }

  template<typename R>
  const R& CommitHandler<R>::get(std::size_t i) const noexcept {
    return m_children[i].m_reactor;
  }

  template<typename R>
  R& CommitHandler<R>::get(std::size_t i) noexcept {
    return m_children[i].m_reactor;
  }
}

#endif

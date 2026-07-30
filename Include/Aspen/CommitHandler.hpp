#ifndef ASPEN_COMMIT_HANDLER_HPP
#define ASPEN_COMMIT_HANDLER_HPP
#include <atomic>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"

namespace Aspen {

  /**
   * Helper class used to commit a list of reactors and evaluate to their
   * aggregate state.
   * @param <R> The type of reactor to manage.
   */
  template<IsReactor R>
  class CommitHandler {
    public:

      /**
       * Constructs a CommitHandler.
       * @param children The reactors whose commits are to be managed.
       */
      template<typename A = std::allocator<R>>
      explicit CommitHandler(std::vector<R, A> children);

      CommitHandler(CommitHandler&& handler) noexcept;

      /**
       * Commits all children and returns their aggregate State.
       * @param sequence The commit's sequence.
       * @return The aggregate State of all children.
       */
      State commit(std::uint64_t sequence) noexcept;

      /** Returns the number of reactors managed. */
      std::size_t size() const noexcept;

      /** Returns the reactor at the specified index. */
      auto& get(this auto&& self, std::size_t index) noexcept;

      /**
       * Returns the indices of the reactors that evaluated during the most
       * recent commit.
       */
      const std::vector<std::size_t>& get_evaluated() const noexcept;

      CommitHandler& operator =(CommitHandler&& handler) noexcept;

    private:
      static constexpr auto BITS = std::size_t(64);
      struct Child {
        CommitFlag m_flag;
        [[no_unique_address]]
        R m_reactor;
        State m_state;
        bool m_has_evaluation;

        template<typename U> requires std::constructible_from<R, U>
        explicit Child(U&& reactor);
        Child(Child&& child) noexcept;
      };
      std::unique_ptr<std::atomic_uint64_t[]> m_raised;
      std::size_t m_word_count;
      std::vector<Child> m_children;
      std::vector<std::size_t> m_evaluated;
      std::size_t m_completion_count;
      std::size_t m_evaluation_count;
      bool m_is_initializing;
      bool m_is_linked;

      void link() noexcept;
  };

  template<IsReactor R>
  template<typename U> requires std::constructible_from<R, U>
  CommitHandler<R>::Child::Child(U&& reactor)
    : m_reactor(std::forward<U>(reactor)),
      m_state(State::NONE),
      m_has_evaluation(false) {}

  template<IsReactor R>
  CommitHandler<R>::Child::Child(Child&& child) noexcept
    : m_reactor(std::move(child.m_reactor)),
      m_state(child.m_state),
      m_has_evaluation(child.m_has_evaluation) {}

  template<IsReactor R>
  template<typename A>
  CommitHandler<R>::CommitHandler(std::vector<R, A> children)
      : m_word_count((children.size() + BITS - 1) / BITS),
        m_completion_count(0),
        m_evaluation_count(0),
        m_is_initializing(true),
        m_is_linked(false) {
    m_children.reserve(children.size());
    for(auto& child : children) {
      m_children.emplace_back(std::move(child));
    }
    m_raised = std::make_unique<std::atomic_uint64_t[]>(m_word_count);
  }

  template<IsReactor R>
  CommitHandler<R>::CommitHandler(CommitHandler&& handler) noexcept
    : m_raised(std::move(handler.m_raised)),
      m_word_count(handler.m_word_count),
      m_children(std::move(handler.m_children)),
      m_evaluated(std::move(handler.m_evaluated)),
      m_completion_count(handler.m_completion_count),
      m_evaluation_count(handler.m_evaluation_count),
      m_is_initializing(handler.m_is_initializing),
      m_is_linked(false) {}

  template<IsReactor R>
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

  template<IsReactor R>
  void CommitHandler<R>::link() noexcept {
    m_is_linked = true;
    auto parent = CommitFlag::get_current();
    for(auto i = std::size_t(0); i != m_children.size(); ++i) {
      auto& flag = m_children[i].m_flag;
      flag.set_parent(parent);
      flag.set_slot(&m_raised[i / BITS], static_cast<std::uint8_t>(i % BITS));
    }
  }

  template<IsReactor R>
  State CommitHandler<R>::commit(std::uint64_t sequence) noexcept {
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
        if(is_complete(child.m_state)) {
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

  template<IsReactor R>
  const std::vector<std::size_t>&
      CommitHandler<R>::get_evaluated() const noexcept {
    return m_evaluated;
  }

  template<IsReactor R>
  std::size_t CommitHandler<R>::size() const noexcept {
    return m_children.size();
  }

  template<IsReactor R>
  auto& CommitHandler<R>::get(this auto&& self, std::size_t index) noexcept {
    return self.m_children[index].m_reactor;
  }
}

#endif

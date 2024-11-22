export module Aspen:CommitHandler;

import <utility>;
import <vector>;
import :State;

export namespace Aspen {

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

    private:
      struct Child {
        R m_reactor;
        State m_state;
        bool m_has_evaluation;

        Child(R reactor);
      };
      std::vector<Child> m_children;
      bool m_is_initializing;
  };

  template<typename R>
  CommitHandler<R>::Child::Child(R reactor)
    : m_reactor(std::move(reactor)),
      m_state(State::NONE),
      m_has_evaluation(false) {}

  template<typename R>
  template<typename A>
  CommitHandler<R>::CommitHandler(std::vector<R, A> children)
      : m_is_initializing(true) {
    for(auto& child : children) {
      m_children.push_back(std::move(child));
    }
  }

  template<typename R>
  State CommitHandler<R>::commit(int sequence) noexcept {
    if(m_children.empty()) {
      return State::COMPLETE;
    }
    auto state = State::NONE;
    auto evaluation_count = std::size_t(0);
    auto completion_count = std::size_t(0);
    auto has_continue = false;
    for(auto& child : m_children) {
      if(is_complete(child.m_state)) {
        ++completion_count;
        if(m_is_initializing && child.m_has_evaluation) {
          ++evaluation_count;
        }
      } else {
        child.m_state = child.m_reactor.commit(sequence);
        if(m_is_initializing) {
          child.m_has_evaluation |= has_evaluation(child.m_state);
          if(child.m_has_evaluation) {
            ++evaluation_count;
          }
        } else if(has_evaluation(child.m_state)) {
          ++evaluation_count;
        }
        if(is_complete(child.m_state)) {
          ++completion_count;
          if(!child.m_has_evaluation) {
            state = State::COMPLETE;
            break;
          }
        } else {
          has_continue |= has_continuation(child.m_state);
        }
      }
    }
    if(state == State::COMPLETE) {
      return State::COMPLETE;
    }
    if(m_is_initializing) {
      if(evaluation_count == m_children.size()) {
        m_is_initializing = false;
        state = combine(state, State::EVALUATED);
      }
    } else if(evaluation_count != 0) {
      state = combine(state, State::EVALUATED);
    }
    if(completion_count == m_children.size()) {
      state = combine(state, State::COMPLETE);
    } else if(has_continue) {
      state = combine(state, State::CONTINUE);
    }
    return state;
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

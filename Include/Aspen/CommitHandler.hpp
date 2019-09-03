#ifndef ASPEN_COMMIT_HANDLER_HPP
#define ASPEN_COMMIT_HANDLER_HPP
#include <utility>
#include <vector>
#include "Aspen/Box.hpp"
#include "Aspen/State.hpp"

namespace Aspen {

  /** Helper class used to commit a list of reactors and evaluate to their
   *  aggregate state.
   */
  class CommitHandler {
    public:

      /**
       * Constructs a CommitHandler.
       * @param children The reactors whose commits are to be managed.
       */
      explicit CommitHandler(std::vector<Box<void>> children);

      /**
       * Commits all children and returns their aggregate State.
       * @param sequence The commit's sequence.
       * @return The aggregate State of all children.
       */
      State commit(int sequence) noexcept;

    private:
      struct Child {
        Box<void> m_reactor;
        State m_state;
        bool m_has_evaluation;

        Child(Box<void> reactor);
      };
      std::vector<Child> m_children;
      bool m_is_initializing;
  };

  inline CommitHandler::Child::Child(Box<void> reactor)
    : m_reactor(std::move(reactor)),
      m_state(State::EMPTY),
      m_has_evaluation(false) {}

  inline CommitHandler::CommitHandler(std::vector<Box<void>> children)
      : m_is_initializing(true) {
    for(auto& child : children) {
      m_children.emplace_back(std::move(child));
    }
  }

  inline State CommitHandler::commit(int sequence) noexcept {
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
}

#endif

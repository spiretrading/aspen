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

      /** Transfers the state of a CommitHandler.
       * @param handler The handler whose state is to be transferred.
       */
      void transfer(const CommitHandler& handler) noexcept;

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

        Child(Box<void> reactor);
      };
      enum class Status {
        INITIALIZING,
        EVALUATING,
        FINAL
      };
      std::vector<Child> m_children;
      Status m_status;
      State m_state;
  };

  inline CommitHandler::Child::Child(Box<void> reactor)
    : m_reactor(std::move(reactor)),
      m_state(State::NONE) {}

  inline CommitHandler::CommitHandler(std::vector<Box<void>> children)
      : m_status(Status::INITIALIZING),
        m_state(State::NONE) {
    for(auto& child : children) {
      m_children.emplace_back(std::move(child));
    }
  }

  inline void CommitHandler::transfer(const CommitHandler& handler) noexcept {
    for(auto i = std::size_t(0); i != handler.m_children.size(); ++i) {
      m_children[i].m_state = handler.m_children[i].m_state;
    }
  }

  inline State CommitHandler::commit(int sequence) noexcept {
    if(m_status == Status::INITIALIZING) {
      auto initialization_count = std::size_t(0);
      auto completion_count = std::size_t(0);
      auto has_continue = false;
      for(auto& child : m_children) {
        if(is_complete(child.m_state)) {
          ++completion_count;
        } else if(!has_evaluation(child.m_state)) {
          child.m_state = child.m_reactor.commit(sequence);
          if(is_complete(child.m_state)) {
            if(is_empty(child.m_state)) {
              m_status = Status::FINAL;
              m_state = State::COMPLETE_EMPTY;
              break;
            }
            ++completion_count;
          } else {
            has_continue |= has_continuation(child.m_state);
          }
          if(has_evaluation(child.m_state)) {
            ++initialization_count;
          }
        } else {
          auto state = child.m_reactor.commit(sequence);
          if(is_complete(state)) {
            ++completion_count;
            child.m_state = state;
          } else {
            has_continue |= has_continuation(state);
          }
          if(has_evaluation(child.m_state)) {
            ++initialization_count;
          }
        }
      }
      if(m_state != State::COMPLETE_EMPTY) {
        if(m_children.empty()) {
          m_state = State::COMPLETE_EMPTY;
          m_status = Status::FINAL;
        } else {
          if(completion_count == m_children.size()) {
            m_state = State::COMPLETE;
            if(initialization_count != 0) {
              m_state = combine(m_state, State::EVALUATED);
            }
            m_status = Status::FINAL;
          } else if(initialization_count == m_children.size()) {
            m_state = State::EVALUATED;
            if(has_continue) {
              m_state = combine(m_state, State::CONTINUE);
            }
            m_status = Status::EVALUATING;
          } else {
            m_state = State::NONE;
            if(has_continue) {
              m_state = combine(m_state, State::CONTINUE);
            }
          }
        }
      }
    } else if(m_status == Status::EVALUATING) {
      m_state = State::NONE;
      auto completion_count = std::size_t(0);
      auto has_continue = false;
      for(auto& child : m_children) {
        if(is_complete(child.m_state)) {
          ++completion_count;
        } else {
          child.m_state = child.m_reactor.commit(sequence);
          has_continue |= has_continuation(child.m_state);
          if(has_evaluation(child.m_state)) {
            m_state = State::EVALUATED;
          }
          if(is_complete(child.m_state)) {
            ++completion_count;
          }
        }
      }
      if(completion_count == m_children.size()) {
        m_state = combine(m_state, State::COMPLETE);
      } else if(has_continue) {
        m_state = combine(m_state, State::CONTINUE);
      }
    }
    return m_state;
  }
}

#endif

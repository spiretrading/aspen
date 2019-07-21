#ifndef ASPEN_COMMIT_REACTOR_HPP
#define ASPEN_COMMIT_REACTOR_HPP
#include <utility>
#include <vector>
#include "Aspen/Box.hpp"
#include "Aspen/State.hpp"

namespace Aspen {

  /** Helper class used to commit a list of reactors and evaluate to their
      aggregate state.
   */
  class CommitReactor {
    public:
      using Type = State;

      /**
       * Constructs a commit reactor.
       * @param children The reactors to aggregate.
       */
      CommitReactor(std::vector<Box<void>> children);

      State commit(int sequence);

      Type eval() const;

    private:
      struct Child {
        Box<void> m_reactor;
        State m_state;

        Child(Box<void> reactor);
      };
      enum class Status {
        INITIALIZING,
        EVALUATING
      };
      std::vector<Child> m_children;
      Status m_status;
      int m_previous_sequence;
      State m_state;
  };

  inline CommitReactor::Child::Child(Box<void> reactor)
    : m_reactor(std::move(reactor)),
      m_state(State::UNINITIALIZED) {}

  inline CommitReactor::CommitReactor(std::vector<Box<void>> children)
      : m_status(Status::INITIALIZING),
        m_previous_sequence(-1),
        m_state(State::UNINITIALIZED) {
    for(auto& child : children) {
      m_children.emplace_back(std::move(child));
    }
  }

  inline State CommitReactor::commit(int sequence) {
    if(is_complete(m_state) || sequence == m_previous_sequence) {
      return m_state;
    }
    if(m_status == Status::INITIALIZING) {
      auto initialization_count = std::size_t(0);
      auto completion_count = std::size_t(0);
      for(auto& child : m_children) {
        if(child.m_state == State::UNINITIALIZED) {
          child.m_state = child.m_reactor.commit(sequence);
          if(is_complete(child.m_state)) {
            ++completion_count;
          }
          if(has_evaluation(child.m_state)) {
            ++initialization_count;
          } else if(is_complete(child.m_state)) {
            m_state = State::COMPLETE_EMPTY;
            break;
          }
        } else {
          ++initialization_count;
          auto state = child.m_reactor.commit(sequence);
          if(is_complete(state)) {
            ++completion_count;
            child.m_state = state;
          }
        }
      }
      if(!m_children.empty() && initialization_count == m_children.size()) {
        if(completion_count == m_children.size()) {
          m_state = State::COMPLETE_EVALUATED;
        } else {
          m_state = State::EVALUATED;
          m_status = Status::EVALUATING;
        }
      } else if(completion_count == m_children.size()) {
        m_state = State::COMPLETE_EMPTY;
      }
    } else if(m_status == Status::EVALUATING) {
      m_state = State::NONE;
      auto completion_count = std::size_t(0);
      for(auto& child : m_children) {
        if(is_complete(child.m_state)) {
          ++completion_count;
        } else {
          child.m_state = child.m_reactor.commit(sequence);
          if(has_evaluation(child.m_state)) {
            m_state = State::EVALUATED;
          }
          if(is_complete(child.m_state)) {
            ++completion_count;
          }
        }
      }
      if(completion_count == m_children.size()) {
        m_state = State::COMPLETE_EVALUATED;
      }
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  inline CommitReactor::Type CommitReactor::eval() const {
    return m_state;
  }
}

#endif

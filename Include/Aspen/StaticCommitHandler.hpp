#ifndef ASPEN_STATIC_COMMIT_HANDLER_HPP
#define ASPEN_STATIC_COMMIT_HANDLER_HPP
#include <tuple>
#include <type_traits>
#include <utility>
#include "Aspen/State.hpp"
#include "Aspen/LocalPtr.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Helper class used to commit a list of reactors and evaluate to their
   * aggregate state.
   * @param <R> The types of each reactor to manage.
   */
  template<typename... R>
  class StaticCommitHandler {
    public:

      /** The types of each reactor to manage. */
      using Reactors = std::tuple<R...>;

      /**
       * Constructs a StaticCommitHandler.
       * @param children The reactors whose commits are to be managed.
       */
      template<typename... A>
      explicit StaticCommitHandler(A&&... children);

      /**
       * Constructs a StaticCommitHandler.
       * @param children The reactors whose commits are to be managed.
       */
      template<typename... A>
      explicit StaticCommitHandler(std::tuple<A...> children);

      /**
       * Transfers the state of a StaticCommitHandler.
       * @param handler The handler whose state is to be transferred.
       */
      void transfer(const StaticCommitHandler& handler);

      /**
       * Commits all children and returns their aggregate State.
       * @param sequence The commit's sequence.
       * @return The aggregate State of all children.
       */
      State commit(int sequence);

    private:
      template<typename C>
      struct Child {
        try_ptr_t<C> m_reactor;
        State m_state;

        template<typename CF>
        Child(CF&& reactor);
      };
      template<typename C>
      Child(C&&) -> Child<std::decay_t<C>>;
      enum class Status {
        INITIALIZING,
        EVALUATING,
        FINAL
      };
      std::tuple<Child<R>...> m_children;
      Status m_status;
      State m_state;

      StaticCommitHandler(const StaticCommitHandler&) = delete;
      StaticCommitHandler& operator =(const StaticCommitHandler&) = delete;
  };

  template<typename... A>
  StaticCommitHandler(A&&...) -> StaticCommitHandler<std::decay_t<A>...>;

  template<typename... A>
  StaticCommitHandler(std::tuple<A...>) ->
    StaticCommitHandler<std::decay_t<A>...>;

  template<typename... R>
  template<typename C>
  template<typename CF>
  StaticCommitHandler<R...>::Child<C>::Child(CF&& reactor)
    : m_reactor(std::forward<CF>(reactor)),
      m_state(State::NONE) {}

  template<typename... R>
  template<typename... A>
  StaticCommitHandler<R...>::StaticCommitHandler(A&&... children)
    : m_children(std::forward<A>(children)...),
      m_status(Status::INITIALIZING),
      m_state(State::NONE) {}

  template<typename... R>
  template<typename... A>
  StaticCommitHandler<R...>::StaticCommitHandler(std::tuple<A...> children)
    : m_children(std::apply([] (auto&&... arguments) {
        return std::make_tuple(Child(std::move(arguments))...);
      }, children)),
      m_status(Status::INITIALIZING),
      m_state(State::NONE) {}

  template<typename... R>
  void StaticCommitHandler<R...>::transfer(const StaticCommitHandler& handler) {
    for_each<std::size_t{0}, sizeof...(R)>([&] (auto index) {
      std::get<decltype(index)::value>(m_children).m_state =
        std::get<decltype(index)::value>(handler.m_children).m_state;
    });
  }

  template<typename... R>
  State StaticCommitHandler<R...>::commit(int sequence) {
    if(m_status == Status::INITIALIZING) {
      auto initialization_count = std::size_t(0);
      auto completion_count = std::size_t(0);
      auto has_continue = false;
      for_each(m_children, [&] (auto& child) {
        if(m_state == State::COMPLETE_EMPTY) {
          return;
        }
        if(is_complete(child.m_state)) {
          ++completion_count;
        } else if(!has_evaluation(child.m_state)) {
          child.m_state = child.m_reactor->commit(sequence);
          if(is_complete(child.m_state)) {
            if(is_empty(child.m_state)) {
              m_status = Status::FINAL;
              m_state = State::COMPLETE_EMPTY;
              return;
            }
            ++completion_count;
          } else {
            has_continue |= has_continuation(child.m_state);
          }
          if(has_evaluation(child.m_state)) {
            ++initialization_count;
          }
        } else {
          auto state = child.m_reactor->commit(sequence);
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
      });
      if(m_state != State::COMPLETE_EMPTY) {
        if(sizeof...(R) == 0) {
          m_state = State::COMPLETE_EMPTY;
          m_status = Status::FINAL;
        } else {
          if(completion_count == sizeof...(R)) {
            m_state = State::COMPLETE;
            if(initialization_count != 0) {
              m_state = combine(m_state, State::EVALUATED);
            }
            m_status = Status::FINAL;
          } else if(initialization_count == sizeof...(R)) {
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
      for_each(m_children, [&] (auto& child) {
        if(is_complete(child.m_state)) {
          ++completion_count;
        } else {
          child.m_state = child.m_reactor->commit(sequence);
          has_continue |= has_continuation(child.m_state);
          if(has_evaluation(child.m_state)) {
            m_state = State::EVALUATED;
          }
          if(is_complete(child.m_state)) {
            ++completion_count;
          }
        }
      });
      if(completion_count == sizeof...(R)) {
        m_state = combine(m_state, State::COMPLETE);
      } else if(has_continue) {
        m_state = combine(m_state, State::CONTINUE);
      }
    }
    return m_state;
  }
}

#endif

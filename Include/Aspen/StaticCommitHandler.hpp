#ifndef ASPEN_STATIC_COMMIT_HANDLER_HPP
#define ASPEN_STATIC_COMMIT_HANDLER_HPP
#include <tuple>
#include <type_traits>
#include <utility>
#include "Aspen/State.hpp"
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
      void transfer(const StaticCommitHandler& handler) noexcept;

      /**
       * Commits all children and returns their aggregate State.
       * @param sequence The commit's sequence.
       * @return The aggregate State of all children.
       */
      State commit(int sequence) noexcept;

    private:
      template<typename C>
      struct Child {
        C m_reactor;
        State m_state;
        bool m_has_evaluation;

        template<typename CF>
        Child(CF&& reactor);
      };
      std::tuple<Child<R>...> m_children;
      bool m_is_initializing;

      template<typename CF>
      static auto make_child(CF&& reactor);
      StaticCommitHandler(const StaticCommitHandler&) = delete;
      StaticCommitHandler& operator =(const StaticCommitHandler&) = delete;
  };

  template<typename... A>
  StaticCommitHandler(A&&...) -> StaticCommitHandler<std::decay_t<A>...>;

  template<typename A>
  StaticCommitHandler(A&&) -> StaticCommitHandler<std::decay_t<A>>;

  template<typename A1, typename A2>
  StaticCommitHandler(A1&&, A2&&) -> StaticCommitHandler<std::decay_t<A1>,
    std::decay_t<A2>>;

  template<typename... A>
  StaticCommitHandler(std::tuple<A...>) ->
    StaticCommitHandler<std::decay_t<A>...>;

  template<typename... R>
  template<typename C>
  template<typename CF>
  StaticCommitHandler<R...>::Child<C>::Child(CF&& reactor)
    : m_reactor(std::forward<CF>(reactor)),
      m_state(State::NONE),
      m_has_evaluation(false) {}

  template<typename... R>
  template<typename... A>
  StaticCommitHandler<R...>::StaticCommitHandler(A&&... children)
    : m_children(std::forward<A>(children)...),
      m_is_initializing(true) {}

  template<typename... R>
  template<typename... A>
  StaticCommitHandler<R...>::StaticCommitHandler(std::tuple<A...> children)
    : m_children(std::apply([] (auto&&... arguments) {
        return std::make_tuple(make_child(std::move(arguments))...);
      }, std::move(children))),
      m_is_initializing(true) {}

  template<typename... R>
  void StaticCommitHandler<R...>::transfer(
      const StaticCommitHandler& handler) noexcept {
    for_each<std::size_t{0}, sizeof...(R)>([&] (auto index) noexcept {
      std::get<decltype(index)::value>(m_children).m_state =
        std::get<decltype(index)::value>(handler.m_children).m_state;
      std::get<decltype(index)::value>(m_children).m_has_evaluation =
        std::get<decltype(index)::value>(handler.m_children).m_has_evaluation;
    });
    m_is_initializing = handler.m_is_initializing;
  }

  template<typename... R>
  State StaticCommitHandler<R...>::commit(int sequence) noexcept {
    if(sizeof...(R) == 0) {
      return State::COMPLETE;
    }
    auto state = State::NONE;
    auto evaluation_count = std::size_t(0);
    auto completion_count = std::size_t(0);
    auto has_continue = false;
    for_each(m_children, [&] (auto& child) noexcept {
      if(state == State::COMPLETE) {
        return;
      }
      if(is_complete(child.m_state)) {
        ++completion_count;
      } else {
        child.m_state = child.m_reactor->commit(sequence);
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
          }
        } else {
          has_continue |= has_continuation(child.m_state);
        }
      }
    });
    if(state == State::COMPLETE) {
      return State::COMPLETE;
    }
    if(m_is_initializing) {
      if(evaluation_count == sizeof...(R)) {
        m_is_initializing = false;
        state = combine(state, State::EVALUATED);
      }
    } else if(evaluation_count != 0) {
      state = combine(state, State::EVALUATED);
    }
    if(completion_count == sizeof...(R)) {
      state = combine(state, State::COMPLETE);
    } else if(has_continue) {
      state = combine(state, State::CONTINUE);
    }
    return state;
  }

  template<typename... R>
  template<typename CF>
  auto StaticCommitHandler<R...>::make_child(CF&& reactor) {
    return Child<std::decay_t<CF>>(std::forward<CF>(reactor));
  }
}

#endif

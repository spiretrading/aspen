#ifndef ASPEN_STATIC_COMMIT_HANDLER_HPP
#define ASPEN_STATIC_COMMIT_HANDLER_HPP
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {
namespace Details {
  template<typename F, typename H, std::size_t... I>
  decltype(auto) apply_impl(F&& f, const H& handler,
      std::index_sequence<I...>) {
    return std::invoke(std::forward<F>(f), handler.template get<I>()...);
  }

  template<typename F, typename H, std::size_t... I>
  decltype(auto) apply_impl(F&& f, H& handler, std::index_sequence<I...>) {
    return std::invoke(std::forward<F>(f), handler.template get<I>()...);
  }
}

  /**
   * Helper class used to commit a list of reactors and evaluate to their
   * aggregate state.
   * @param <R> The types of each reactor to manage.
   */
  template<typename... R>
  class StaticCommitHandler {
    public:

      /**
       * Constructs a StaticCommitHandler.
       * @param children The reactors whose commits are to be managed.
       */
      explicit StaticCommitHandler(const R&... children);

      /**
       * Constructs a StaticCommitHandler.
       * @param children The reactors whose commits are to be managed.
       */
      template<typename... A>
      explicit StaticCommitHandler(A&&... children);

      /** Copies a StaticCommitHandler. */
      StaticCommitHandler(const StaticCommitHandler& handler);

      /** Moves a StaticCommitHandler. */
      StaticCommitHandler(StaticCommitHandler&& handler) noexcept;

      /**
       * Commits all children and returns their aggregate State.
       * @param sequence The commit's sequence.
       * @return The aggregate State of all children.
       */
      State commit(int sequence) noexcept;

      /** Returns the reactor at the specified index. */
      template<std::size_t I>
      const std::tuple_element_t<I, std::tuple<R...>>& get() const noexcept;

      /** Returns the reactor at the specified index. */
      template<std::size_t I>
      std::tuple_element_t<I, std::tuple<R...>>& get() noexcept;

      /** Copies a StaticCommitHandler. */
      StaticCommitHandler& operator =(const StaticCommitHandler& handler);

      /** Moves a StaticCommitHandler. */
      StaticCommitHandler& operator =(StaticCommitHandler&& handler) noexcept;

    private:
      template<typename C>
      struct Child {
        CommitFlag m_flag;
        C m_reactor;
        State m_state;
        bool m_has_evaluation;

        template<typename CF, typename = std::enable_if_t<
          !std::is_base_of_v<Child, std::decay_t<CF>>>>
        Child(CF&& reactor);
        Child(const Child& child);
        Child(Child&& child) noexcept;
        Child& operator =(const Child& child);
        Child& operator =(Child&& child) noexcept;
      };
      std::tuple<Child<R>...> m_children;
      bool m_is_initializing;
      bool m_is_linked;

      template<typename CF>
      static auto make_child(CF&& reactor);
  };

  /** Applies a callable on every reactor represented by a
      StaticCommitHandler.
      @param f The callable to apply.
      @param handler The handler to apply the callable to.
   */
  template<typename F, typename... A>
  decltype(auto) apply(F&& f, const StaticCommitHandler<A...>& handler) {
    return Details::apply_impl(std::forward<F>(f), handler,
      std::make_index_sequence<sizeof...(A)>{});
  }

  /** Applies a callable on every reactor represented by a
      StaticCommitHandler.
      @param f The callable to apply.
      @param handler The handler to apply the callable to.
   */
  template<typename F, typename... A>
  decltype(auto) apply(F&& f, StaticCommitHandler<A...>& handler) {
    return Details::apply_impl(std::forward<F>(f), handler,
      std::make_index_sequence<sizeof...(A)>{});
  }

  template<typename... A>
  StaticCommitHandler(A&&...) -> StaticCommitHandler<std::decay_t<A>...>;

  template<typename A>
  StaticCommitHandler(A&&) -> StaticCommitHandler<std::decay_t<A>>;

  template<typename A1, typename A2>
  StaticCommitHandler(A1&&, A2&&) -> StaticCommitHandler<std::decay_t<A1>,
    std::decay_t<A2>>;

  template<typename... R>
  template<typename C>
  template<typename CF, typename>
  StaticCommitHandler<R...>::Child<C>::Child(CF&& reactor)
    : m_reactor(std::forward<CF>(reactor)),
      m_state(State::NONE),
      m_has_evaluation(false) {}

  template<typename... R>
  template<typename C>
  StaticCommitHandler<R...>::Child<C>::Child(const Child& child)
    : m_reactor(child.m_reactor),
      m_state(child.m_state),
      m_has_evaluation(child.m_has_evaluation) {}

  template<typename... R>
  template<typename C>
  StaticCommitHandler<R...>::Child<C>::Child(Child&& child) noexcept
    : m_reactor(std::move(child.m_reactor)),
      m_state(child.m_state),
      m_has_evaluation(child.m_has_evaluation) {}

  template<typename... R>
  template<typename C>
  typename StaticCommitHandler<R...>::template Child<C>&
      StaticCommitHandler<R...>::Child<C>::operator =(const Child& child) {
    m_reactor = child.m_reactor;
    m_state = child.m_state;
    m_has_evaluation = child.m_has_evaluation;
    m_flag.raise();
    return *this;
  }

  template<typename... R>
  template<typename C>
  typename StaticCommitHandler<R...>::template Child<C>&
      StaticCommitHandler<R...>::Child<C>::operator =(Child&& child) noexcept {
    m_reactor = std::move(child.m_reactor);
    m_state = child.m_state;
    m_has_evaluation = child.m_has_evaluation;
    m_flag.raise();
    return *this;
  }

  template<typename... R>
  StaticCommitHandler<R...>::StaticCommitHandler(const R&... children)
    : m_children(children...),
      m_is_initializing(true),
      m_is_linked(false) {}

  template<typename... R>
  template<typename... A>
  StaticCommitHandler<R...>::StaticCommitHandler(A&&... children)
    : m_children(std::forward<A>(children)...),
      m_is_initializing(true),
      m_is_linked(false) {}

  template<typename... R>
  StaticCommitHandler<R...>::StaticCommitHandler(
    const StaticCommitHandler& handler)
    : m_children(handler.m_children),
      m_is_initializing(handler.m_is_initializing),
      m_is_linked(false) {}

  template<typename... R>
  StaticCommitHandler<R...>::StaticCommitHandler(
    StaticCommitHandler&& handler) noexcept
    : m_children(std::move(handler.m_children)),
      m_is_initializing(handler.m_is_initializing),
      m_is_linked(false) {}

  template<typename... R>
  StaticCommitHandler<R...>& StaticCommitHandler<R...>::operator =(
      const StaticCommitHandler& handler) {
    m_children = handler.m_children;
    m_is_initializing = handler.m_is_initializing;
    m_is_linked = false;
    return *this;
  }

  template<typename... R>
  StaticCommitHandler<R...>& StaticCommitHandler<R...>::operator =(
      StaticCommitHandler&& handler) noexcept {
    m_children = std::move(handler.m_children);
    m_is_initializing = handler.m_is_initializing;
    m_is_linked = false;
    return *this;
  }

  template<typename... R>
  State StaticCommitHandler<R...>::commit(int sequence) noexcept {
    if(sizeof...(R) == 0) {
      return State::COMPLETE;
    }
    if(!m_is_linked) {
      m_is_linked = true;
      auto parent = CommitFlag::get_current();
      for_each(m_children, [&] (auto& child) noexcept {
        child.m_flag.set_parent(parent);
      });
    }
    auto state = State::NONE;
    auto evaluation_count = std::size_t(0);
    auto completion_count = std::size_t(0);
    auto has_continue = false;
    auto is_initializing = m_is_initializing;
    for_each(m_children, [&] (auto& child) noexcept {
      if(state == State::COMPLETE) {
        return;
      }
      if(is_complete(child.m_state)) {
        ++completion_count;
        if(is_initializing && child.m_has_evaluation) {
          ++evaluation_count;
        }
      } else if(!child.m_flag.is_raised()) {
        child.m_state = State::NONE;
        if(is_initializing && child.m_has_evaluation) {
          ++evaluation_count;
        }
      } else {
        child.m_flag.clear();
        {
          auto scope = CommitFlagScope(child.m_flag);
          child.m_state = child.m_reactor.commit(sequence);
        }
        if(is_initializing) {
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
          if(has_continuation(child.m_state)) {
            child.m_flag.raise();
          }
        }
      }
    });
    if(state == State::COMPLETE) {
      return State::COMPLETE;
    }
    if(is_initializing) {
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
  template<std::size_t I>
  std::tuple_element_t<I, std::tuple<R...>>&
      StaticCommitHandler<R...>::get() noexcept {
    return std::get<I>(m_children).m_reactor;
  }

  template<typename... R>
  template<std::size_t I>
  const std::tuple_element_t<I, std::tuple<R...>>&
      StaticCommitHandler<R...>::get() const noexcept {
    return std::get<I>(m_children).m_reactor;
  }

  template<typename... R>
  template<typename CF>
  auto StaticCommitHandler<R...>::make_child(CF&& reactor) {
    return Child<std::decay_t<CF>>(std::forward<CF>(reactor));
  }
}

#endif

#ifndef ASPEN_CONCAT_HPP
#define ASPEN_CONCAT_HPP
#include <concepts>
#include <cstdint>
#include <iterator>
#include <list>
#include <optional>
#include <type_traits>
#include <utility>
#include "Aspen/Branch.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to every value produced by its
   * children.
   * @param <R> The type of reactor producing the reactors to evaluate to.
   */
  template<IsReactor R> requires IsReactor<reactor_result_t<R>>
  class Concat {
    public:

      /** The type of reactor producing the reactors to evaluate to. */
      using Reactor = R;

      /** The type to evaluate to. */
      using Type = reactor_result_t<reactor_result_t<Reactor>>;

      /** The type returned by an evaluation. */
      using Result = common_evaluation_t<reactor_result_t<Reactor>>;

      /** Whether an evaluation is noexcept. */
      static constexpr auto is_noexcept =
        is_noexcept_evaluation_v<reactor_result_t<Reactor>>;

      /**
       * Constructs a Concat.
       * @param producer The reactor producing the reactors to evaluate to.
       */
      template<typename RF> requires std::constructible_from<R, RF>
      explicit Concat(RF&& producer);

      State commit(std::uint64_t sequence) noexcept;
      Result eval() const noexcept(is_noexcept);

    private:
      std::optional<Branch<Reactor>> m_producer;
      std::list<Branch<reactor_result_t<Reactor>>> m_children;
      bool m_is_child_complete;
  };

  template<typename R> requires(
    !std::derived_from<std::remove_cvref_t<R>, Concat<to_reactor_t<R>>>)
  Concat(R&&) -> Concat<to_reactor_t<R>>;

  /**
   * Concats the reactors produced by its child.
   * @param producer The reactor producing the reactors to evaluate to.
   * @return A reactor evaluating to every value the <i>producer</i> produces.
   */
  template<typename R> requires IsReactor<std::remove_cvref_t<R>> &&
    IsReactor<reactor_result_t<R>>
  auto concat(R&& producer) {
    return Concat(std::forward<R>(producer));
  }

  template<IsReactor R> requires IsReactor<reactor_result_t<R>>
  template<typename RF> requires std::constructible_from<R, RF>
  Concat<R>::Concat(RF&& producer)
    : m_producer(std::forward<RF>(producer)),
      m_is_child_complete(false) {}

  template<IsReactor R> requires IsReactor<reactor_result_t<R>>
  State Concat<R>::commit(std::uint64_t sequence) noexcept {
    auto state = [&] {
      if(m_producer) {
        auto producer_state = m_producer->commit(sequence);
        if(has_evaluation(producer_state)) {
          try {
            m_children.emplace_back((*m_producer)->eval());
          } catch(...) {}
        }
        if(has_continuation(producer_state)) {
          return State::CONTINUE;
        }
        if(is_complete(producer_state)) {
          m_producer = std::nullopt;
          if(m_children.empty() ||
              m_children.size() == 1 && m_is_child_complete) {
            return State::COMPLETE;
          }
        }
      }
      return State::NONE;
    }();
    auto child_state = [&] {
      while(true) {
        if(m_is_child_complete) {
          while(m_children.size() > 1) {
            auto next_child = std::next(m_children.begin());
            auto child_state = next_child->commit(sequence);
            if(has_evaluation(child_state)) {
              m_is_child_complete = is_complete(child_state);
              m_children.pop_front();
              return child_state;
            } else if(is_complete(child_state)) {
              m_children.erase(next_child);
            } else if(has_continuation(child_state)) {
              return State::CONTINUE;
            } else {
              break;
            }
          }
          return State::NONE;
        } else if(!m_children.empty()) {
          auto child_state = m_children.front().commit(sequence);
          m_is_child_complete = is_complete(child_state);
          if(!m_is_child_complete || has_evaluation(child_state)) {
            return child_state;
          }
        } else {
          return State::NONE;
        }
      }
    }();
    if(has_evaluation(child_state)) {
      state = combine(state, State::EVALUATED);
      if(m_is_child_complete && m_children.size() > 1) {
        state = combine(state, State::CONTINUE);
      }
    }
    if(has_continuation(child_state)) {
      state = combine(state, State::CONTINUE);
    } else if(m_is_child_complete && m_children.size() == 1 &&
        !m_producer) {
      state = combine(state, State::COMPLETE);
    }
    return state;
  }

  template<IsReactor R> requires IsReactor<reactor_result_t<R>>
  typename Concat<R>::Result Concat<R>::eval() const noexcept(is_noexcept) {
    return m_children.front()->eval();
  }
}

#endif

#ifndef ASPEN_GROUP_HPP
#define ASPEN_GROUP_HPP
#include <concepts>
#include <cstdint>
#include <utility>
#include "Aspen/Branch.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates two children concurrently.
   * @param <A> The first reactor's type.
   * @param <B> The second reactor's type.
   */
  template<IsReactor A, IsReactorOf<reactor_result_t<A>> B>
  class Group {
    public:

      /** The type to evaluate to. */
      using Type = reactor_result_t<A>;

      /** The type returned by an evaluation. */
      using Result = common_evaluation_t<A, B>;

      /** Whether an evaluation is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_evaluation_v<A, B>;

      /**
       * Constructs a Group.
       * @param first The first reactor to commit.
       * @param second The second reactor to commit.
       */
      template<typename AF, typename BF> requires
        std::constructible_from<A, AF> && std::constructible_from<B, BF>
      Group(AF&& first, BF&& second);

      State commit(std::uint64_t sequence) noexcept;
      Result eval() const noexcept(is_noexcept);

    private:
      Branch<A> m_first;
      Branch<B> m_second;
      std::uint8_t m_completions;
      std::uint8_t m_current;
      std::uint8_t m_position;

      std::uint8_t next_position() const noexcept;
      bool is_child_complete(std::uint8_t position) const noexcept;
      bool are_children_complete() const noexcept;
  };

  template<typename A, typename B>
  Group(A&&, B&&) -> Group<to_reactor_t<A>, to_reactor_t<B>>;

  /**
   * Groups two reactors together to be evaluated concurrently.
   * @param first The first reactor to commit.
   * @param second The second reactor to commit.
   * @return A reactor evaluating both of them concurrently.
   */
  template<typename A, typename B> requires IsReactor<to_reactor_t<A>> &&
    IsReactorOf<to_reactor_t<B>, reactor_result_t<A>>
  auto group(A&& first, B&& second) {
    return Group(std::forward<A>(first), std::forward<B>(second));
  }

  /**
   * Groups a series of reactors together.
   * @param first The first reactor to commit.
   * @param second The second reactor to commit.
   * @param remainder The reactors to commit thereafter.
   * @return A reactor evaluating all of them concurrently.
   */
  template<typename A, typename B, typename... C> requires
    IsReactor<to_reactor_t<A>> &&
    IsReactorOf<to_reactor_t<B>, reactor_result_t<A>>
  auto group(A&& first, B&& second, C&&... remainder) {
    return Group(std::forward<A>(first),
      group(std::forward<B>(second), std::forward<C>(remainder)...));
  }

  template<IsReactor A, IsReactorOf<reactor_result_t<A>> B>
  template<typename AF, typename BF> requires
    std::constructible_from<A, AF> && std::constructible_from<B, BF>
  Group<A, B>::Group(AF&& first, BF&& second)
    : m_first(std::forward<AF>(first)),
      m_second(std::forward<BF>(second)),
      m_completions(0),
      m_current(0),
      m_position(0) {}

  template<IsReactor A, IsReactorOf<reactor_result_t<A>> B>
  State Group<A, B>::commit(std::uint64_t sequence) noexcept {
    if(m_position != m_current && is_child_complete(m_position)) {
      m_position = next_position();
    }
    auto state = State::NONE;
    if(!are_children_complete()) {
      auto start = m_position;
      while(true) {
        if(is_child_complete(m_position)) {
          m_position = next_position();
        } else {
          auto child_state = [&] {
            if(m_position == 0) {
              return m_first.commit(sequence);
            }
            return m_second.commit(sequence);
          }();
          if(has_continuation(child_state)) {
            state = combine(state, State::CONTINUE);
          } else if(is_complete(child_state)) {
            m_completions |= 1 << m_position;
          }
          if(has_evaluation(child_state)) {
            state = combine(state, State::EVALUATED);
            if(!is_child_complete(next_position())) {
              state = combine(state, State::CONTINUE);
            }
            m_current = m_position;
            m_position = next_position();
            break;
          } else {
            m_position = next_position();
          }
        }
        if(m_position == start) {
          break;
        }
      }
    }
    if(are_children_complete()) {
      state = combine(state, State::COMPLETE);
    }
    return state;
  }

  template<IsReactor A, IsReactorOf<reactor_result_t<A>> B>
  typename Group<A, B>::Result Group<A, B>::eval() const noexcept(is_noexcept) {
    if(m_current == 0) {
      return m_first->eval();
    }
    return m_second->eval();
  }

  template<IsReactor A, IsReactorOf<reactor_result_t<A>> B>
  std::uint8_t Group<A, B>::next_position() const noexcept {
    return (m_position + 1) % 2;
  }

  template<IsReactor A, IsReactorOf<reactor_result_t<A>> B>
  bool Group<A, B>::is_child_complete(std::uint8_t position) const noexcept {
    return m_completions & (1 << position);
  }

  template<IsReactor A, IsReactorOf<reactor_result_t<A>> B>
  bool Group<A, B>::are_children_complete() const noexcept {
    return is_child_complete(0) && is_child_complete(1);
  }
}

#endif

#ifndef ASPEN_SWITCH_HPP
#define ASPEN_SWITCH_HPP
#include <concepts>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>
#include "Aspen/Branch.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that can be used to determine when a child reactor is
   * committed and evaluated.
   * @param <T> The type of reactor used to determine whether to
   *            commit/evaluate.
   * @param <S> The type of reactor to commit/evaluate to based on the toggle.
   */
  template<IsReactor T, IsReactor S> requires
    std::convertible_to<reactor_result_t<T>, bool>
  class Switch {
    public:

      /** The type to evaluate to. */
      using Type = reactor_result_t<S>;

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_reactor_v<S>;

      /**
       * Constructs a Switch reactor.
       * @param toggle The reactor used to determine whether to commit and
       *        evaluate.
       * @param series The reactor to commit/evaluate based on whether the
       *        <i>toggle</i> evaluates to <code>true</code>.
       */
      template<typename TF, typename SF> requires
        std::constructible_from<T, TF> && std::constructible_from<S, SF>
      Switch(TF&& toggle, SF&& series);

      State commit(std::uint64_t sequence) noexcept;
      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      Branch<T> m_toggle;
      Branch<S> m_series;
      bool m_is_toggle_complete;
      bool m_has_evaluation;
      bool m_is_on;
      std::conditional_t<is_noexcept, std::optional<Type>, Maybe<Type>> m_value;
  };

  template<typename T, typename S>
  Switch(T&&, S&&) -> Switch<to_reactor_t<T>, to_reactor_t<S>>;

  /**
   * Switches a series on and off according to a toggle.
   * @param toggle The reactor used to determine whether to commit and
   *        evaluate.
   * @param series The reactor to commit/evaluate based on whether the
   *        <i>toggle</i> evaluates to <code>true</code>.
   * @return A reactor evaluating to the <i>series</i> while the <i>toggle</i>
   *         evaluates to <code>true</code>.
   */
  template<typename T, typename S> requires IsReactor<to_reactor_t<T>> &&
    IsReactor<to_reactor_t<S>>
  auto switch_(T&& toggle, S&& series) {
    return Switch(std::forward<T>(toggle), std::forward<S>(series));
  }

  template<IsReactor T, IsReactor S> requires
    std::convertible_to<reactor_result_t<T>, bool>
  template<typename TF, typename SF> requires
    std::constructible_from<T, TF> && std::constructible_from<S, SF>
  Switch<T, S>::Switch(TF&& toggle, SF&& series)
    : m_toggle(std::forward<TF>(toggle)),
      m_series(std::forward<SF>(series)),
      m_is_toggle_complete(false),
      m_has_evaluation(false),
      m_is_on(false) {}

  template<IsReactor T, IsReactor S> requires
    std::convertible_to<reactor_result_t<T>, bool>
  State Switch<T, S>::commit(std::uint64_t sequence) noexcept {
    auto was_off = !m_is_on;
    auto toggle_state = State::NONE;
    if(!m_is_toggle_complete) {
      toggle_state = m_toggle.commit(sequence);
      if(has_evaluation(toggle_state)) {
        if constexpr(is_noexcept_reactor_v<T>) {
          m_is_on = m_toggle->eval();
        } else {
          try {
            m_is_on = m_toggle->eval();
          } catch(...) {
            m_is_on = false;
          }
        }
      }
      m_is_toggle_complete = is_complete(toggle_state);
    }
    auto state = State::NONE;
    if(m_is_on) {
      state = m_series.commit(sequence);
      if(has_evaluation(state) || m_has_evaluation && was_off) {
        try_assign(m_value, *m_series);
        state = combine(state, State::EVALUATED);
        m_has_evaluation = true;
      }
    } else if(m_is_toggle_complete) {
      state = combine(state, State::COMPLETE);
    }
    if(has_continuation(toggle_state)) {
      state = combine(state, State::CONTINUE);
    }
    return state;
  }

  template<IsReactor T, IsReactor S> requires
    std::convertible_to<reactor_result_t<T>, bool>
  eval_result_t<typename Switch<T, S>::Type> Switch<T, S>::eval()
      const noexcept(is_noexcept) {
    return *m_value;
  }
}

#endif

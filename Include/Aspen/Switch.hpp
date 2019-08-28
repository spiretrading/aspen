#ifndef ASPEN_SWITCH_HPP
#define ASPEN_SWITCH_HPP
#include <optional>
#include <type_traits>
#include <utility>
#include "Aspen/Maybe.hpp"
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
  template<typename T, typename S>
  class Switch {
    public:
      using Type = reactor_result_t<S>;
      static constexpr auto is_noexcept = is_noexcept_reactor_v<T> &&
        is_noexcept_reactor_v<S>;

      /**
       * Constructs a Switch reactor.
       * @param toggle The reactor used to determine whether to commit and
       *        evaluate.
       * @param series The reactor to commit/evaluate based on whether the
       *        <i>toggle</i> evaluates to <code>true</code>.
       */
      template<typename TF, typename SF>
      Switch(TF&& toggle, SF&& series);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      try_ptr_t<T> m_toggle;
      try_ptr_t<S> m_series;
      State m_toggle_state;
      bool m_is_on;
      std::conditional_t<is_noexcept, std::optional<Type>, Maybe<Type>> m_value;
      State m_state;
      int m_previous_sequence;
  };

  template<typename T, typename S>
  Switch(T&&, S&&) -> Switch<to_reactor_t<T>, to_reactor_t<S>>;

  template<typename T, typename S>
  auto switch_(T&& trigger, S&& series) {
    return Switch(std::forward<T>(trigger), std::forward<S>(series));
  }

  template<typename T, typename S>
  template<typename TF, typename SF>
  Switch<T, S>::Switch(TF&& toggle, SF&& series)
    : m_toggle(std::forward<TF>(toggle)),
      m_series(std::forward<SF>(series)),
      m_toggle_state(State::EMPTY),
      m_is_on(false),
      m_state(State::EMPTY),
      m_previous_sequence(-1) {}

  template<typename T, typename S>
  State Switch<T, S>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    if(is_complete(m_toggle_state)) {
      m_state = m_series->commit(sequence);
    } else {
      auto toggle_state = m_toggle->commit(sequence);
      auto flipped = !m_is_on;
      if(has_evaluation(toggle_state) ||
          is_empty(m_toggle_state) && !is_empty(toggle_state)) {
        try {
          m_is_on = m_toggle->eval();
        } catch(const std::exception&) {
          m_is_on = false;
          if constexpr(!is_noexcept) {
            m_value = std::current_exception();
          }
        }
      }
      m_toggle_state = toggle_state;
      if(m_is_on) {
        auto series_state = m_series->commit(sequence);
        if(has_evaluation(series_state) ||
            (is_empty(m_state) || flipped) && !is_empty(series_state)) {
          m_value = try_eval(*m_series);
          m_state = combine(series_state, State::EVALUATED);
        } else if(!is_empty(m_state)) {
          m_state = series_state;
        } else if(has_continuation(series_state)) {
          m_state = State::CONTINUE_EMPTY;
        } else {
          m_state = State::EMPTY;
        }
        if(is_complete(series_state)) {
          m_state = combine(m_state, State::COMPLETE);
        }
      } else if(is_complete(m_toggle_state)) {
        if(is_empty(m_state)) {
          m_state = State::COMPLETE_EMPTY;
        } else {
          m_state = State::COMPLETE;
        }
      } else if(has_continuation(m_toggle_state)) {
        if(is_empty(m_state)) {
          m_state = State::CONTINUE_EMPTY;
        } else {
          m_state = State::CONTINUE;
        }
      } else if(!is_empty(m_state)) {
        m_state = State::NONE;
      } else {
        m_state = State::EMPTY;
      }
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename T, typename S>
  eval_result_t<typename Switch<T, S>::Type> Switch<T, S>::eval()
      const noexcept(is_noexcept) {
    return *m_value;
  }
}

#endif

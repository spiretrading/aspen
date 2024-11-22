export module Aspen:Switch;

import <optional>;
import <type_traits>;
import <utility>;
import :Maybe;
import :State;
import :Traits;

export namespace Aspen {

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
      T m_toggle;
      S m_series;
      bool m_is_toggle_complete;
      bool m_has_evaluation;
      bool m_is_on;
      std::conditional_t<is_noexcept, std::optional<Type>, Maybe<Type>> m_value;
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
      m_is_toggle_complete(false),
      m_has_evaluation(false),
      m_is_on(false) {}

  template<typename T, typename S>
  State Switch<T, S>::commit(int sequence) noexcept {
    if(m_is_toggle_complete && m_is_on) {
      return m_series.commit(sequence);
    }
    auto toggle_state = m_toggle.commit(sequence);
    auto flipped = !m_is_on;
    if(has_evaluation(toggle_state)) {
      try {
        m_is_on = m_toggle.eval();
      } catch(...) {
        m_is_on = false;
        if constexpr(!is_noexcept) {
          m_value = std::current_exception();
        }
      }
    }
    m_is_toggle_complete = is_complete(toggle_state);
    auto state = State::NONE;
    if(m_is_on) {
      state = m_series.commit(sequence);
      if(has_evaluation(state) || m_has_evaluation && flipped) {
        m_value = try_eval(m_series);
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

  template<typename T, typename S>
  eval_result_t<typename Switch<T, S>::Type> Switch<T, S>::eval()
      const noexcept(is_noexcept) {
    return *m_value;
  }
}

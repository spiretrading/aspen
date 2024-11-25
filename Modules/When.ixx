export module Aspen:When;

import std;
import :State;
import :Traits;

export namespace Aspen {

  /**
   * Implements a reactor that begins evaluating its child when a condition is
   * true.
   * @param <C> The type of reactor used as the condition.
   * @param <T> The type of reactor to evaluate to.
   */
  template<typename C, typename T>
  class When {
    public:
      using Type = reactor_result_t<T>;
      static constexpr auto is_noexcept = is_noexcept_reactor_v<C> &&
        is_noexcept_reactor_v<T>;

      /**
       * Constructs a When reactor.
       * @param condition The condition to evaluate.
       * @param series The series to evaluate when the condition evaluates to
       *        <code>true</i>.
       */
      template<typename CF, typename TF>
      When(CF&& condition, TF&& series);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      C m_condition;
      T m_series;
      bool m_is_condition_complete;
      bool m_is_triggered;
  };

  template<typename C, typename T>
  When(C&&, T&&) -> When<to_reactor_t<C>, to_reactor_t<T>>;

  /**
   * Returns a reactor that begins evaluating its child when a condition is
   * true.
   * @param condition The condition to evaluate.
   * @param The series to evaluate when the condition evaluates to
   *        <code>true</i>.
   */
  template<typename C, typename T>
  auto when(C&& condition, T&& series) {
    return When(std::forward<C>(condition), std::forward<T>(series));
  }

  template<typename C, typename T>
  template<typename CF, typename TF>
  When<C, T>::When(CF&& condition, TF&& series)
    : m_condition(std::forward<CF>(condition)),
      m_series(std::forward<TF>(series)),
      m_is_condition_complete(false),
      m_is_triggered(false) {}

  template<typename C, typename T>
  State When<C, T>::commit(int sequence) noexcept {
    auto state = State::NONE;
    if(!m_is_triggered) {
      auto condition_state = m_condition.commit(sequence);
      if(has_evaluation(condition_state)) {
        try {
          m_is_triggered = m_condition.eval();
        } catch(...) {}
      }
      m_is_condition_complete = is_complete(condition_state);
      if(!m_is_triggered) {
        if(m_is_condition_complete) {
          state = combine(state, State::COMPLETE);
        } else if(has_continuation(condition_state)) {
          state = combine(state, State::CONTINUE);
        }
      }
    }
    if(m_is_triggered) {
      auto series_state = m_series.commit(sequence);
      if(has_evaluation(series_state)) {
        state = combine(state, State::EVALUATED);
      }
      if(is_complete(series_state)) {
        state = combine(state, State::COMPLETE);
      } else if(has_continuation(series_state)) {
        state = combine(state, State::CONTINUE);
      }
    }
    return state;
  }

  template<typename C, typename T>
  eval_result_t<typename When<C, T>::Type> When<C, T>::eval() const noexcept(
      is_noexcept) {
    return m_series.eval();
  }
}

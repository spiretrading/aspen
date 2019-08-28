#ifndef ASPEN_WHEN_HPP
#define ASPEN_WHEN_HPP
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

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
      try_ptr_t<C> m_condition;
      try_ptr_t<T> m_series;
      State m_condition_state;
      bool m_is_triggered;
      State m_state;
      int m_previous_sequence;
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
      m_condition_state(State::EMPTY),
      m_is_triggered(false),
      m_state(State::EMPTY),
      m_previous_sequence(-1) {}

  template<typename C, typename T>
  State When<C, T>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    if(is_empty(m_state)) {
      m_state = State::EMPTY;
    } else {
      m_state = State::NONE;
    }
    if(!m_is_triggered) {
      auto condition_state = m_condition->commit(sequence);
      if(has_evaluation(condition_state) ||
          is_empty(m_condition_state) && !is_empty(condition_state)) {
        try {
          m_is_triggered = m_condition->eval();
        } catch(const std::exception&) {}
      }
      if(!m_is_triggered) {
        if(is_complete(condition_state)) {
          m_state = combine(m_state, State::COMPLETE);
        } else if(has_continuation(condition_state)) {
          m_state = combine(m_state, State::CONTINUE);
        }
      }
      m_condition_state = condition_state;
    }
    if(m_is_triggered) {
      auto series_state = m_series->commit(sequence);
      if(has_evaluation(series_state) ||
          is_empty(m_state) && !is_empty(series_state)) {
        m_state = combine(m_state, State::EVALUATED);
      }
      if(is_complete(series_state)) {
        m_state = combine(m_state, State::COMPLETE);
      } else if(has_continuation(series_state)) {
        m_state = combine(m_state, State::CONTINUE);
      }
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename C, typename T>
  eval_result_t<typename When<C, T>::Type> When<C, T>::eval() const noexcept(
      is_noexcept) {
    return m_series->eval();
  }
}

#endif

#ifndef ASPEN_UNTIL_HPP
#define ASPEN_UNTIL_HPP
#include <optional>
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that commits its child until a condition is reached.
   * @param <C> The type of reactor used as the condition.
   * @param <T> The type of reactor to evaluate to.
   */
  template<typename C, typename T>
  class Until {
    public:
      using Type = reactor_result_t<T>;
      static constexpr auto is_noexcept = is_noexcept_reactor_v<C> &&
        is_noexcept_reactor_v<T>;

      /**
       * Constructs an Until.
       * @param condition The condition to evaluate.
       * @param series The series to evaluate to until the <i>condition</i> is
       *        reached.
       */
      template<typename CF, typename TF>
      Until(CF&& condition, TF&& series);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      C m_condition;
      std::optional<T> m_series;
      try_maybe_t<Type, !is_noexcept> m_value;
      bool m_is_condition_complete;
  };

  template<typename C, typename T>
  Until(C&&, T&&) -> Until<to_reactor_t<C>, to_reactor_t<T>>;

  /**
   * Returns a reactor that commits its child until a condition is reached.
   * @param condition The condition to evaluate.
   * @param series The series to evaluate to until the <i>condition</i> is
   *        reached.
   */
  template<typename C, typename T>
  auto until(C&& condition, T&& series) {
    return Until(std::forward<C>(condition), std::forward<T>(series));
  }

  template<typename C, typename T>
  template<typename CF, typename TF>
  Until<C, T>::Until(CF&& condition, TF&& series)
    : m_condition(std::forward<CF>(condition)),
      m_series(std::forward<TF>(series)),
      m_is_condition_complete(false) {}

  template<typename C, typename T>
  State Until<C, T>::commit(int sequence) noexcept {
    auto state = State::NONE;
    auto has_condition_continuation = false;
    if(!m_is_condition_complete) {
      auto condition_state = m_condition.commit(sequence);
      if(has_evaluation(condition_state)) {
        try {
          if(m_condition.eval()) {
            m_series = std::nullopt;
            state = State::COMPLETE;
          }
        } catch(const std::exception&) {
          if constexpr(!is_noexcept) {
            m_value = std::current_exception();
          }
        }
      }
      has_condition_continuation = has_continuation(condition_state);
    }
    if(m_series.has_value()) {
      auto series_state = m_series->commit(sequence);
      if(has_evaluation(series_state)) {
        m_value = try_eval(*m_series);
        state = State::EVALUATED;
      }
      if(is_complete(series_state)) {
        state = combine(state, State::COMPLETE);
      } else if(has_condition_continuation || has_continuation(series_state)) {
        state = combine(state, State::CONTINUE);
      }
    }
    return state;
  }

  template<typename C, typename T>
  eval_result_t<typename Until<C, T>::Type> Until<C, T>::eval() const noexcept(
      is_noexcept) {
    return *m_value;
  }
}

#endif

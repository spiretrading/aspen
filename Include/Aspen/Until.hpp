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

      const Type& eval() const noexcept(is_noexcept);

    private:
      try_ptr_t<C> m_condition;
      std::optional<T> m_series;
      try_maybe_t<Type, is_noexcept> m_value;
      State m_condition_state;
      State m_state;
      int m_previous_sequence;
  };

  template<typename C, typename T>
  Until(C&&, T&&) -> Until<std::decay_t<C>, std::decay_t<T>>;

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
      m_condition_state(State::EMPTY),
      m_state(State::EMPTY),
      m_previous_sequence(-1) {}

  template<typename C, typename T>
  State Until<C, T>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    if(!is_complete(m_condition_state)) {
      m_condition_state = m_condition->commit(sequence);
      if(has_evaluation(m_condition_state) ||
          is_empty(m_state) && !is_empty(m_condition_state)) {
        auto condition = try_eval(*m_condition);
        try {
          if(*condition) {
            m_series = std::nullopt;
            if(is_empty(m_state)) {
              m_state = State::COMPLETE_EMPTY;
            } else {
              m_state = State::COMPLETE;
            }
          }
        } catch(const std::exception&) {
          m_value = std::current_exception();
        }
      }
    }
    if(m_series.has_value()) {
      auto series_state = m_series->commit(sequence);
      if(has_evaluation(series_state) ||
          is_empty(m_state) && !is_empty(series_state)) {
        m_value = try_eval(*m_series);
      }
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename C, typename T>
  const typename Until<C, T>::Type& Until<C, T>::eval() const noexcept(
      is_noexcept) {
    return *m_value;
  }
}

#endif

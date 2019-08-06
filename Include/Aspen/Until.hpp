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
      State m_state;
      int m_previous_sequence;
  };

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
      m_series(std::forward<TF>(series)) {}

  template<typename C, typename T>
  State Until<C, T>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    auto condition_state = m_condition->commit(sequence);
    if(has_evaluation(condition_state) || 
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

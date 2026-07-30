#ifndef ASPEN_WHEN_HPP
#define ASPEN_WHEN_HPP
#include <concepts>
#include <cstdint>
#include <utility>
#include "Aspen/Branch.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that begins evaluating its child when a condition is
   * true.
   * @param <C> The type of reactor used as the condition.
   * @param <T> The type of reactor to evaluate to.
   */
  template<IsReactor C, IsReactor T> requires
    std::convertible_to<reactor_result_t<C>, bool>
  class When {
    public:

      /** The type to evaluate to. */
      using Type = reactor_result_t<T>;

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_reactor_v<T>;

      /**
       * Constructs a When reactor.
       * @param condition The condition to evaluate.
       * @param series The series to evaluate when the condition evaluates to
       *        <code>true</code>.
       */
      template<typename CF, typename TF> requires
        std::constructible_from<C, CF> && std::constructible_from<T, TF>
      When(CF&& condition, TF&& series);

      State commit(std::uint64_t sequence) noexcept;
      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      Branch<C> m_condition;
      Branch<T> m_series;
      bool m_is_triggered;
  };

  template<typename C, typename T>
  When(C&&, T&&) -> When<to_reactor_t<C>, to_reactor_t<T>>;

  /**
   * Returns a reactor that begins evaluating its child when a condition is
   * true.
   * @param condition The condition to evaluate.
   * @param series The series to evaluate when the condition evaluates to
   *        <code>true</code>.
   * @return A reactor evaluating to the <i>series</i> once the
   *         <i>condition</i> evaluates to <code>true</code>.
   */
  template<typename C, typename T> requires IsReactor<to_reactor_t<C>> &&
    IsReactor<to_reactor_t<T>>
  auto when(C&& condition, T&& series) {
    return When(std::forward<C>(condition), std::forward<T>(series));
  }

  template<IsReactor C, IsReactor T> requires
    std::convertible_to<reactor_result_t<C>, bool>
  template<typename CF, typename TF> requires
    std::constructible_from<C, CF> && std::constructible_from<T, TF>
  When<C, T>::When(CF&& condition, TF&& series)
    : m_condition(std::forward<CF>(condition)),
      m_series(std::forward<TF>(series)),
      m_is_triggered(false) {}

  template<IsReactor C, IsReactor T> requires
    std::convertible_to<reactor_result_t<C>, bool>
  State When<C, T>::commit(std::uint64_t sequence) noexcept {
    auto state = State::NONE;
    if(!m_is_triggered) {
      auto condition_state = m_condition.commit(sequence);
      if(has_evaluation(condition_state)) {
        m_is_triggered = [&] {
          if constexpr(is_noexcept_reactor_v<C>) {
            return static_cast<bool>(m_condition->eval());
          } else {
            try {
              return static_cast<bool>(m_condition->eval());
            } catch(...) {
              return false;
            }
          }
        }();
      }
      if(!m_is_triggered) {
        if(is_complete(condition_state)) {
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

  template<IsReactor C, IsReactor T> requires
    std::convertible_to<reactor_result_t<C>, bool>
  eval_result_t<typename When<C, T>::Type> When<C, T>::eval()
      const noexcept(is_noexcept) {
    return m_series->eval();
  }
}

#endif

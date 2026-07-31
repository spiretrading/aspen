#ifndef ASPEN_UNTIL_HPP
#define ASPEN_UNTIL_HPP
#include <concepts>
#include <cstdint>
#include <optional>
#include <utility>
#include "Aspen/Branch.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that commits its child until a condition is reached.
   * @param <C> The type of reactor used as the condition.
   * @param <T> The type of reactor to evaluate to.
   */
  template<IsReactorOf<bool> C, IsReactor T>
  class Until {
    public:

      /** The type to evaluate to. */
      using Type = reactor_result_t<T>;

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_reactor_v<T>;

      /**
       * Constructs an Until.
       * @param condition The condition to evaluate.
       * @param series The series to evaluate to until the <i>condition</i> is
       *        reached.
       */
      template<typename CF, typename TF> requires
        std::constructible_from<C, CF> && std::constructible_from<T, TF>
      Until(CF&& condition, TF&& series);

      State commit(std::uint64_t sequence) noexcept;
      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      std::optional<Branch<C>> m_condition;
      std::optional<Branch<T>> m_series;
      try_maybe_t<Type, !is_noexcept> m_value;
  };

  template<typename C, typename T>
  Until(C&&, T&&) -> Until<to_reactor_t<C>, to_reactor_t<T>>;

  /**
   * Returns a reactor that commits its child until a condition is reached.
   * @param condition The condition to evaluate.
   * @param series The series to evaluate to until the <i>condition</i> is
   *        reached.
   * @return A reactor evaluating to the <i>series</i> until the
   *         <i>condition</i> is reached.
   */
  template<typename C, typename T> requires
    IsReactorOf<to_reactor_t<C>, bool> && IsReactor<to_reactor_t<T>>
  auto until(C&& condition, T&& series) {
    return Until(std::forward<C>(condition), std::forward<T>(series));
  }

  template<IsReactorOf<bool> C, IsReactor T>
  template<typename CF, typename TF> requires
    std::constructible_from<C, CF> && std::constructible_from<T, TF>
  Until<C, T>::Until(CF&& condition, TF&& series)
    : m_condition(std::forward<CF>(condition)),
      m_series(std::forward<TF>(series)) {}

  template<IsReactorOf<bool> C, IsReactor T>
  State Until<C, T>::commit(std::uint64_t sequence) noexcept {
    auto state = State::NONE;
    auto has_condition_continuation = false;
    if(m_condition) {
      auto condition_state = m_condition->commit(sequence);
      if(has_evaluation(condition_state)) {
        auto is_reached = [&] {
          if constexpr(is_noexcept_reactor_v<C>) {
            return static_cast<bool>((*m_condition)->eval());
          } else {
            try {
              return static_cast<bool>((*m_condition)->eval());
            } catch(...) {
              return false;
            }
          }
        }();
        if(is_reached) {
          m_series = std::nullopt;
          state = State::COMPLETE;
        }
      }
      has_condition_continuation = has_continuation(condition_state);
      if(is_complete(condition_state) || !m_series) {
        m_condition = std::nullopt;
      }
    }
    if(m_series) {
      auto series_state = m_series->commit(sequence);
      if(has_evaluation(series_state)) {
        try_assign(m_value, **m_series);
        state = combine(state, State::EVALUATED);
      }
      if(is_complete(series_state)) {
        state = combine(state, State::COMPLETE);
        m_series = std::nullopt;
        m_condition = std::nullopt;
      } else if(has_condition_continuation || has_continuation(series_state)) {
        state = combine(state, State::CONTINUE);
      }
    }
    return state;
  }

  template<IsReactorOf<bool> C, IsReactor T>
  eval_result_t<typename Until<C, T>::Type> Until<C, T>::eval()
      const noexcept(is_noexcept) {
    return *m_value;
  }
}

#endif

#ifndef ASPEN_SWITCH_HPP
#define ASPEN_SWITCH_HPP
#include <optional>
#include <type_traits>
#include <utility>
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {
  template<typename T, typename S>
  class Switch {
    public:
      using Type = reactor_result_t<S>;

      static constexpr auto is_noexcept = is_noexcept_reactor_v<T> &&
        is_noexcept_reactor_v<S>;

      template<typename TF, typename SF>
      Switch(TF&& toggle, SF&& series);

      State commit(int sequence) noexcept;

      const Type& eval() const noexcept(is_noexcept);

    private:
      try_ptr_t<T> m_toggle;
      try_ptr_t<S> m_series;
      bool m_is_toggle_complete;
      bool m_is_on;
      std::conditional_t<is_noexcept, std::optional<Type>, Maybe<Type>> m_value;
      State m_state;
      int m_previous_sequence;
      bool m_had_evaluation;
  };

  template<typename T, typename S>
  Switch(T&&, S&&) -> Switch<std::decay_t<T>, std::decay_t<S>>;

  template<typename T, typename S>
  template<typename TF, typename SF>
  Switch<T, S>::Switch(TF&& toggle, SF&& series)
    : m_toggle(std::forward<TF>(toggle)),
      m_series(std::forward<SF>(series)),
      m_is_toggle_complete(false),
      m_is_on(false),
      m_state(State::NONE),
      m_previous_sequence(-1),
      m_had_evaluation(false) {}

  template<typename T, typename S>
  State Switch<T, S>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    if(m_is_toggle_complete) {
      m_state = m_series->commit(sequence);
    } else {
      m_state = State::NONE;
      auto toggle_state = m_toggle->commit(sequence);
      if(has_evaluation(toggle_state)) {
        if constexpr(is_noexcept_reactor_v<T>) {
          m_is_on = m_toggle->eval();
        } else {
          try {
            m_is_on = m_toggle->eval();
          } catch(const std::exception&) {
            m_is_on = false;
            m_value = std::current_exception();
          }
        }
      }
      m_is_toggle_complete = is_complete(toggle_state);
      if(m_is_on) {
        auto series_state = m_series->commit(sequence);
        if(has_evaluation(series_state)) {
          m_value = try_eval(*m_series);
        }
      }
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename T, typename S>
  const typename Switch<T, S>::Type& Switch<T, S>::eval()
      const noexcept(is_noexcept) {
    if(m_is_toggle_complete && m_is_on) {
      return m_series->eval();
    }
    return *m_value;
  }
}

#endif

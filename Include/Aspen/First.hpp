#ifndef ASPEN_FIRST_HPP
#define ASPEN_FIRST_HPP
#include <type_traits>
#include "Aspen/LocalPtr.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to the first value it receives from its
   * source.
   * @param <T> The type of the source reactor.
   */
  template<typename T>
  class First {
    public:
      using Type = reactor_result_t<T>;

      static constexpr auto is_noexcept = is_noexcept_reactor_v<T>;

      /**
       * Constructs a First.
       * @param series The reactor to evaluate.
       */
      template<typename TF>
      First(TF&& series);

      State commit(int sequence) noexcept;

      const Type& eval() const noexcept(is_noexcept);

    private:
      try_ptr_t<T> m_series;
      State m_state;
      try_maybe_t<Type, !is_noexcept> m_value;
      int m_previous_sequence;
  };

  template<typename T>
  First(T&&) -> First<std::decay_t<T>>;

  /** Returns a reactor evaluating to the first value evaluated. */
  template<typename Reactor>
  auto first(Reactor&& source) {
    return First(std::forward<Reactor>(source));
  }

  template<typename T>
  template<typename TF>
  First<T>::First(TF&& series)
    : m_series(std::forward<TF>(series)),
      m_state(State::EMPTY),
      m_previous_sequence(-1) {}

  template<typename T>
  State First<T>::commit(int sequence) noexcept {
    if(is_complete(m_state) || sequence == m_previous_sequence) {
      return m_state;
    }
    auto state = m_series->commit(sequence);
    if(has_evaluation(state)) {
      m_state = State::COMPLETE_EVALUATED;
      m_value = try_eval(*m_series);
    } else if(has_continuation(state)) {
      m_state = State::CONTINUE_EMPTY;
    } else if(is_complete(state)) {
      m_state = State::COMPLETE_EMPTY;
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename T>
  const typename First<T>::Type& First<T>::eval() const noexcept(is_noexcept) {
    return *m_value;
  }
}

#endif

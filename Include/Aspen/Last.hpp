#ifndef ASPEN_LAST_HPP
#define ASPEN_LAST_HPP
#include <type_traits>
#include "Aspen/LocalPtr.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to the last value it receives from its
   * source.
   * @param <T> The type of the source reactor.
   */
  template<typename T>
  class Last {
    public:
      using Type = reactor_result_t<T>;

      static constexpr auto is_noexcept = is_noexcept_reactor_v<T>;

      /**
       * Constructs a Last.
       * @param series The reactor to evaluate.
       */
      template<typename TF>
      Last(TF&& series);

      State commit(int sequence) noexcept;

      const Type& eval() const noexcept(is_noexcept);

    private:
      try_ptr_t<T> m_series;
      State m_state;
      try_maybe_t<Type, !is_noexcept> m_value;
      int m_previous_sequence;
  };

  template<typename T>
  Last(T&&) -> Last<std::decay_t<T>>;

  /** Returns a reactor evaluating to the last value evaluated. */
  template<typename Reactor>
  auto last(Reactor&& source) {
    return Last(std::forward<Reactor>(source));
  }

  template<typename T>
  template<typename TF>
  Last<T>::Last(TF&& series)
    : m_series(std::forward<TF>(series)),
      m_state(State::EMPTY),
      m_previous_sequence(-1) {}

  template<typename T>
  State Last<T>::commit(int sequence) noexcept {
    if(is_complete(m_state) || sequence == m_previous_sequence) {
      return m_state;
    }
    auto state = m_series->commit(sequence);
    if(is_complete(state)) {
      if(is_empty(state)) {
        m_state = State::COMPLETE_EMPTY;
      } else {
        m_value = try_eval(*m_series);
        m_state = State::COMPLETE_EVALUATED;
      }
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename T>
  const typename Last<T>::Type& Last<T>::eval() const noexcept(is_noexcept) {
    return *m_value;
  }
}

#endif

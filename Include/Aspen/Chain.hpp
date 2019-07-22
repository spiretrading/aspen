#ifndef ASPEN_CHAIN_HPP
#define ASPEN_CHAIN_HPP
#include <utility>
#include "Aspen/LocalPtr.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that produces values from one reactor until it
   * completes and then produces values from another reactor.
   * @param <A> The first reactor's type.
   * @param <B> The second reactor's type.
   */
  template<typename A, typename B>
  class Chain {
    public:
      using Type = reactor_result_t<A>;

      /**
       * Constructs a Chain.
       * @param initial The reactor to initially evaluate to.
       * @param continuation The reactor to evaluate to thereafter.
       */
      template<typename AF, typename BF>
      Chain(AF&& initial, BF&& continuation);

      State commit(int sequence);

      const Type& eval() const;

    private:
      enum class Status {
        INITIAL,
        TRANSITIONING,
        CONTINUATION
      };
      try_ptr_t<A> m_initial;
      try_ptr_t<B> m_continuation;
      Status m_status;
      State m_state;
      int m_previous_sequence;
      bool m_had_evaluation;
  };

  template<typename A, typename B>
  Chain(A&&, B&&) -> Chain<std::decay_t<A>, std::decay_t<B>>;

  /**
    * Chains two reactors together so that one runs after the other.
    * @param initial The reactor to initially evaluate to.
    * @param continuation The reactor to evaluate to thereafter.
    */
  template<typename A, typename B>
  auto chain(A&& a, B&& b) {
    return Chain(std::forward<A>(a), std::forward<B>(b));
  }

  template<typename A, typename B>
  template<typename AF, typename BF>
  Chain<A, B>::Chain(AF&& initial, BF&& continuation)
    : m_initial(std::forward<AF>(initial)),
      m_continuation(std::forward<BF>(continuation)),
      m_status(Status::INITIAL),
      m_state(State::NONE),
      m_previous_sequence(-1),
      m_had_evaluation(false) {}

  template<typename A, typename B>
  State Chain<A, B>::commit(int sequence) {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    if(m_status == Status::INITIAL) {
      auto update = m_initial->commit(sequence);
      if(update == State::COMPLETE_EVALUATED) {
        m_status = Status::TRANSITIONING;
        m_state = State::EVALUATED;
      } else if(update == State::COMPLETE_EMPTY) {
        m_status = Status::CONTINUATION;
      } else {
        m_state = update;
      }
    } else if(m_status == Status::TRANSITIONING) {
      m_status = Status::CONTINUATION;
    }
    if(m_status == Status::CONTINUATION) {
      auto update = m_continuation->commit(sequence);
      if(update == State::COMPLETE_EMPTY) {
        m_status = Status::INITIAL;
        if(m_had_evaluation) {
          m_state = State::COMPLETE;
        } else {
          m_state = update;
        }
      } else {
        m_state = update;
      }
    }
    m_previous_sequence = sequence;
    m_had_evaluation |= has_evaluation(m_state);
    return m_state;
  }

  template<typename A, typename B>
  const typename Chain<A, B>::Type& Chain<A, B>::eval() const {
    if(m_status != Status::CONTINUATION) {
      return m_initial->eval();
    } else {
      return m_continuation->eval();
    }
  }
}

#endif
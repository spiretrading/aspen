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

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const;

    private:
      enum class Status {
        INITIAL,
        TRANSITIONING,
        CONTINUATION,
        PROXY_CONTINUATION
      };
      try_ptr_t<A> m_initial;
      try_ptr_t<B> m_continuation;
      Status m_status;
      State m_state;
      int m_previous_sequence;
  };

  template<typename A, typename B>
  Chain(A&&, B&&) -> Chain<to_reactor_t<A>, to_reactor_t<B>>;

  /**
   * Chains two reactors together so that one runs after the other.
   * @param initial The reactor to initially evaluate to.
   * @param continuation The reactor to evaluate to thereafter.
   */
  template<typename A, typename B>
  auto chain(A&& initial, B&& continuation) {
    return Chain(std::forward<A>(initial), std::forward<B>(continuation));
  }

  /**
   * Chains a series of reactors together.
   * @param initial The reactor to initially evaluate to.
   * @param continuation The reactor to evaluate to thereafter.
   */
  template<typename A, typename B, typename... C>
  auto chain(A&& initial, B&& continuation, C&&... remainder) {
    return Chain(std::forward<A>(initial),
      chain(std::forward<B>(continuation), std::forward<C>(remainder)...));
  }

  template<typename A, typename B>
  template<typename AF, typename BF>
  Chain<A, B>::Chain(AF&& initial, BF&& continuation)
    : m_initial(std::forward<AF>(initial)),
      m_continuation(std::forward<BF>(continuation)),
      m_status(Status::INITIAL),
      m_state(State::EMPTY),
      m_previous_sequence(-1) {}

  template<typename A, typename B>
  State Chain<A, B>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    if(m_status == Status::INITIAL) {
      auto update = m_initial->commit(sequence);
      if(is_complete(update)) {
        if(is_empty(update)) {
          m_status = Status::PROXY_CONTINUATION;
        } else if(has_evaluation(update) || is_empty(m_state)) {
          m_status = Status::TRANSITIONING;
          m_state = State::CONTINUE_EVALUATED;
        } else {
          m_status = Status::CONTINUATION;
        }
      } else if(!is_empty(update) && is_empty(m_state)) {
        m_state = set(update, State::EVALUATED);
      } else {
        m_state = update;
      }
    } else if(m_status == Status::TRANSITIONING) {
      m_status = Status::CONTINUATION;
    }
    if(m_status == Status::PROXY_CONTINUATION) {
      m_state = m_continuation->commit(sequence);
    } else if(m_status == Status::CONTINUATION) {
      auto update = m_continuation->commit(sequence);
      if(update == State::COMPLETE_EMPTY) {
        m_status = Status::INITIAL;
        if(is_empty(m_state)) {
          m_state = State::COMPLETE_EMPTY;
        } else {
          m_state = State::COMPLETE;
        }
      } else if(is_empty(update)) {
        if(has_continuation(update)) {
          m_state = State::CONTINUE;
        } else {
          m_state = State::NONE;
        }
      } else {
        m_state = update;
      }
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename A, typename B>
  eval_result_t<typename Chain<A, B>::Type> Chain<A, B>::eval() const {
    if(m_status == Status::INITIAL ||
        m_status == Status::TRANSITIONING) {
      return m_initial->eval();
    } else {
      return m_continuation->eval();
    }
  }
}

#endif

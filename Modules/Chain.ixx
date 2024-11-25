export module Aspen:Chain;

import std;
import :State;
import :Traits;

export namespace Aspen {

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
      static constexpr auto is_noexcept = is_noexcept_reactor_v<A> &&
        is_noexcept_reactor_v<B>;

      /**
       * Constructs a Chain.
       * @param initial The reactor to initially evaluate to.
       * @param continuation The reactor to evaluate to thereafter.
       */
      template<typename AF, typename BF>
      Chain(AF&& initial, BF&& continuation);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      enum class Status : char {
        START,
        INITIAL,
        TRANSITIONING,
        CONTINUATION
      };
      A m_initial;
      B m_continuation;
      Status m_status;
      std::uint8_t m_which;
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
      m_status(Status::START),
      m_which(0) {}

  template<typename A, typename B>
  State Chain<A, B>::commit(int sequence) noexcept {
    if(m_status == Status::START) {
      auto state = m_initial.commit(sequence);
      if(has_evaluation(state)) {
        if(is_complete(state)) {
          m_status = Status::TRANSITIONING;
          return State::CONTINUE_EVALUATED;
        }
        m_status = Status::INITIAL;
        return state;
      } else if(is_complete(state)) {
        m_status = Status::CONTINUATION;
        m_which = 1;
        return m_continuation.commit(sequence);
      } else {
        return state;
      }
    } else if(m_status == Status::INITIAL) {
      auto state = m_initial.commit(sequence);
      if(is_complete(state)) {
        m_status = Status::TRANSITIONING;
        if(has_evaluation(state)) {
          return State::CONTINUE_EVALUATED;
        }
      } else {
        return state;
      }
    } else if(m_status == Status::CONTINUATION) {
      return m_continuation.commit(sequence);
    }
    auto state = m_continuation.commit(sequence);
    if(has_evaluation(state)) {
      m_status = Status::CONTINUATION;
      m_which = 1;
    }
    return state;
  }

  template<typename A, typename B>
  eval_result_t<typename Chain<A, B>::Type> Chain<A, B>::eval()
      const noexcept(is_noexcept){
    if(m_which == 0) {
      return m_initial.eval();
    } else {
      return m_continuation.eval();
    }
  }
}

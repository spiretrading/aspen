export module Aspen:Weak;

import <memory>;
import <optional>;
import :Shared;
import :State;
import :Traits;

export namespace Aspen {

  /**
   * Implements a weak reference to an existing shared reactor.
   * @param <R> The type of reactor to observe.
   */
  template<typename R>
  class Weak {
    public:
      using Reactor = R;
      using Type = reactor_result_t<Reactor>;
      using Result = decltype(std::declval<Reactor>().eval());
      static constexpr auto is_noexcept = is_noexcept_reactor_v<Reactor>;

      /**
       * Constructs a Weak reactor observing an existing Shared reactor.
       * @param reactor The reactor to observe.
       */
      explicit Weak(Shared<Reactor> reactor) noexcept;

      /** Returns a new Shared reactor to the reactor being observed. */
      std::optional<Shared<Reactor>> lock() const noexcept;

      State commit(int sequence) noexcept;

      Result eval() const noexcept(is_noexcept);

    private:
      std::shared_ptr<Details::SharedEvaluator<Reactor>> m_evaluator;
      int m_last_evaluation;
  };

  template<typename R>
  Weak<R>::Weak(Shared<Reactor> reactor) noexcept
    : m_evaluator(std::move(reactor.m_evaluator)),
      m_last_evaluation(-1) {}

  template<typename R>
  std::optional<Shared<R>> Weak<R>::lock() const noexcept {
    auto reactor = m_evaluator->m_reactor.lock();
    if(reactor == nullptr) {
      return std::nullopt;
    }
    return Shared(m_evaluator, std::move(reactor));
  }

  template<typename R>
  State Weak<R>::commit(int sequence) noexcept {
    auto reactor = m_evaluator->m_reactor.lock();
    if(reactor == nullptr) {
      if(m_last_evaluation < m_evaluator->m_state->m_last_evaluation) {
        return State::COMPLETE_EVALUATED;
      }
      return State::COMPLETE;
    } else {
      return Shared<Reactor>::commit_state(sequence, *reactor, *m_evaluator,
        m_last_evaluation);
    }
  }

  template<typename R>
  typename Weak<R>::Result Weak<R>::eval() const noexcept(is_noexcept) {
    if(m_evaluator->m_evaluation.has_value()) {
      return *m_evaluator->m_evaluation;
    }
    auto reactor = m_evaluator->m_reactor.lock();
    return reactor->eval();
  }
}

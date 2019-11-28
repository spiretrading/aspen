#ifndef ASPEN_WEAK_HPP
#define ASPEN_WEAK_HPP
#include <memory>
#include "Aspen/Shared.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

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

      State commit(int sequence) noexcept;

      Result eval() const noexcept(is_noexcept);

    private:
      std::shared_ptr<Reactor> m_reactor;
      std::weak_ptr<Details::SharedState> m_state;
      bool m_is_empty;
  };

  template<typename R>
  Weak<R>::Weak(Shared<Reactor> reactor) noexcept
    : m_reactor(std::move(reactor.m_reactor)),
      m_state(std::move(reactor.m_state)),
      m_is_empty(true) {}

  template<typename R>
  State Weak<R>::commit(int sequence) noexcept {
    auto state = m_state.lock();
    if(state == nullptr) {
      return State::COMPLETE;
    }
    return Shared<R>::commit_state(sequence, *state, *m_reactor, m_is_empty);
  }

  template<typename R>
  typename Weak<R>::Result Weak<R>::eval() const noexcept(is_noexcept) {
    return m_reactor->eval();
  }
}

#endif

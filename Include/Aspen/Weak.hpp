#ifndef ASPEN_WEAK_HPP
#define ASPEN_WEAK_HPP
#include <optional>
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

      Weak(const Weak& weak) noexcept;

      Weak(Weak&& weak) = default;

      /** Returns a new Shared reactor to the reactor being observed. */
      std::optional<Shared<R>> lock() const noexcept;

      State commit(int sequence) noexcept;

      Result eval() const noexcept(is_noexcept);

      Weak& operator =(const Weak& weak) noexcept;

      Weak& operator =(Weak&& weak) = default;

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
  Weak<R>::Weak(const Weak& weak) noexcept
    : m_reactor(weak.m_reactor),
      m_state(weak.m_state),
      m_is_empty(true) {}

  template<typename R>
  std::optional<Shared<R>> Weak<R>::lock() const noexcept {
    auto state = m_state.lock();
    if(state == nullptr) {
      return std::nullopt;
    }
    return Shared<Reactor>(state, m_reactor);
  }

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

  template<typename R>
  Weak<R>& Weak<R>::operator =(const Weak& weak) noexcept {
    m_reactor = weak.m_reactor;
    m_state = weak.m_state;
    m_is_empty = true;
    return *this;
  }
}

#endif

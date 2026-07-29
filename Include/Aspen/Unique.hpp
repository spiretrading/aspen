#ifndef ASPEN_UNIQUE_HPP
#define ASPEN_UNIQUE_HPP
#include <cstdint>
#include <memory>
#include <utility>
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Used to hold a unique reactor, can not be shared with multiple reactors.
   * @param <R> The type of reactor to own.
   */
  template<IsReactor R>
  class Unique {
    public:

      /** The type of reactor being owned. */
      using Reactor = R;

      /** The type to evaluate to. */
      using Type = reactor_result_t<Reactor>;

      /** The type returned by an evaluation. */
      using Result = decltype(std::declval<Reactor&>().eval());

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_reactor_v<Reactor>;

      /** Constructs an uninitialized reactor. */
      Unique() = default;

      /**
       * Constructs a Unique reactor that owns its argument.
       * @param reactor The reactor to own.
       */
      explicit Unique(Reactor* reactor) noexcept;

      /**
       * Constructs a Unique reactor that owns its argument.
       * @param reactor The reactor to own.
       */
      explicit Unique(std::unique_ptr<Reactor> reactor) noexcept;

      const Reactor& operator *() const noexcept;
      const Reactor* operator ->() const noexcept;
      Reactor& operator *() noexcept;
      Reactor* operator ->() noexcept;
      State commit(std::uint64_t sequence) noexcept;
      Result eval() const noexcept(is_noexcept);

    private:
      template<IsReactor> friend class Shared;
      std::unique_ptr<Reactor> m_reactor;
  };

  template<IsReactor R>
  Unique<R>::Unique(Reactor* reactor) noexcept
    : Unique(std::unique_ptr<Reactor>(reactor)) {}

  template<IsReactor R>
  Unique<R>::Unique(std::unique_ptr<Reactor> reactor) noexcept
    : m_reactor(std::move(reactor)) {}

  template<IsReactor R>
  const typename Unique<R>::Reactor& Unique<R>::operator *() const noexcept {
    return *m_reactor;
  }

  template<IsReactor R>
  const typename Unique<R>::Reactor* Unique<R>::operator ->() const noexcept {
    return m_reactor.get();
  }

  template<IsReactor R>
  typename Unique<R>::Reactor& Unique<R>::operator *() noexcept {
    return *m_reactor;
  }

  template<IsReactor R>
  typename Unique<R>::Reactor* Unique<R>::operator ->() noexcept {
    return m_reactor.get();
  }

  template<IsReactor R>
  State Unique<R>::commit(std::uint64_t sequence) noexcept {
    return m_reactor->commit(sequence);
  }

  template<IsReactor R>
  typename Unique<R>::Result Unique<R>::eval() const noexcept(is_noexcept) {
    return m_reactor->eval();
  }
}

#endif

#ifndef ASPEN_UNIQUE_HPP
#define ASPEN_UNIQUE_HPP
#include <memory>
#include <utility>
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Used to hold a unique reactor, can not be shared with multiple reactors.
   * @param <R> The type of reactor to own.
   */
  template<typename R>
  class Unique {
    public:
      using Reactor = R;
      using Type = reactor_result_t<Reactor>;
      using Result = decltype(std::declval<Reactor>().eval());
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

      //! Returns a reference to the reactor.
      const Reactor& operator *() const noexcept;

      //! Returns a pointer to the reactor.
      const Reactor* operator ->() const noexcept;

      //! Returns a reference to the reactor.
      Reactor& operator *() noexcept;

      //! Returns a pointer to the reactor.
      Reactor* operator ->() noexcept;

      State commit(int sequence) noexcept;

      Result eval() const noexcept(is_noexcept);

    private:
      std::unique_ptr<Reactor> m_reactor;
  };

  template<typename R>
  Unique<R>::Unique(Reactor* reactor) noexcept
    : m_reactor(reactor) {}

  template<typename R>
  Unique<R>::Unique(std::unique_ptr<Reactor> reactor) noexcept
    : m_reactor(std::move(reactor)) {}

  template<typename R>
  const typename Unique<R>::Reactor& Unique<R>::operator *() const noexcept {
    return *m_reactor;
  }

  template<typename R>
  const typename Unique<R>::Reactor* Unique<R>::operator ->() const noexcept {
    return m_reactor.get();
  }

  template<typename R>
  typename Unique<R>::Reactor& Unique<R>::operator *() noexcept {
    return *m_reactor;
  }

  template<typename R>
  typename Unique<R>::Reactor* Unique<R>::operator ->() noexcept {
    return m_reactor.get();
  }

  template<typename R>
  State Unique<R>::commit(int sequence) noexcept {
    return m_reactor->commit(sequence);
  }

  template<typename R>
  typename Unique<R>::Result Unique<R>::eval() const noexcept(is_noexcept) {
    return m_reactor->eval();
  }
}

#endif

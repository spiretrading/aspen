#ifndef ASPEN_SHARED_HPP
#define ASPEN_SHARED_HPP
#include <memory>
#include <utility>
#include "Aspen/Box.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"
#include "Aspen/Unique.hpp"

namespace Aspen {
namespace Details {
  struct SharedState {
    State m_state;
    bool m_is_empty;
    int m_sequence;

    SharedState();
  };

  inline SharedState::SharedState()
    : m_state(State::NONE),
      m_is_empty(true),
      m_sequence(-1) {}
}

  /**
   * Used to share a reactor as a child among multiple reactors.
   * @param <R> The type of reactor being shared.
   */
  template<typename R>
  class Shared {
    public:
      using Reactor = R;
      using Type = reactor_result_t<Reactor>;
      using Result = decltype(std::declval<Reactor>().eval());
      static constexpr auto is_noexcept = is_noexcept_reactor_v<Reactor>;

      /** Constructs a Shared reactor. */
      Shared();

      /**
       * Constructs a Shared reactor.
       * @param args The argument used to emplace the shared reactor.
       */
      template<typename A, typename = std::enable_if_t<
        !std::is_base_of_v<Shared, std::decay_t<A>>>>
      explicit Shared(A&& args);

      /**
       * Constructs a Shared reactor.
       * @param args The arguments used to emplace the shared reactor.
       */
      template<typename A, typename... B, typename = std::enable_if_t<
        !std::is_base_of_v<Shared, std::decay_t<A>>>>
      explicit Shared(A&& a, B&&... args);

      /**
       * Constructs a Shared reactor from a Unique reactor.
       * @param reactor The reactor to transfer ownership from.
       */
      Shared(Unique<Reactor> reactor);

      /**
       * Constructs a Shared reactor from an existing Shared reactor.
       * @param reactor The reactor to share ownership with.
       */
      template<typename U>
      Shared(Shared<U> reactor);

      Shared(const Shared& shared);

      Shared(Shared&& shared) = default;

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

      Shared& operator =(const Shared& shared);

      Shared& operator =(Shared&& shared) = default;

    private:
      template<typename> friend class Shared;
      template<typename> friend class Weak;
      std::shared_ptr<Details::SharedState> m_state;
      std::shared_ptr<Reactor> m_reactor;
      bool m_is_empty;

      static State commit_state(int sequence, Details::SharedState& state,
        Reactor& reactor, bool& is_empty);
  };

  /** Type alias for a Shared<Box<T>>. */
  template<typename T>
  using SharedBox = Shared<Box<T>>;

  /**
   * Boxes a reactor into a copyable generic interface.
   * @param reactor The reactor to wrap.
   */
  template<typename R>
  auto shared_box(R&& reactor) {
    return SharedBox<reactor_result_t<R>>(std::forward<R>(reactor));
  }

  /**
   * A type trait that provides the type Shared<T> if T is not already wrapped
   * in a Shared, otherwise provides the type T.
   */
  template<typename T>
  struct collapse_shared {
    using type = Shared<T>;
  };

  template<typename T>
  struct collapse_shared<Shared<T>> {
    using type = typename collapse_shared<T>::type;
  };

  template<typename T>
  using collapse_shared_t = typename collapse_shared<T>::type;

  template<typename A, typename = std::enable_if_t<
    !std::is_base_of_v<Shared<to_reactor_t<A>>, std::decay_t<A>>>>
  Shared(A&&) -> Shared<to_reactor_t<A>>;

  template<typename R>
  Shared<R>::Shared()
    : m_state(std::make_shared<Details::SharedState>()),
      m_reactor(std::make_shared<Reactor>()),
      m_is_empty(true) {}

  template<typename R>
  template<typename A, typename>
  Shared<R>::Shared(A&& args)
    : m_state(std::make_shared<Details::SharedState>()),
      m_reactor(std::make_shared<Reactor>(std::forward<A>(args))),
      m_is_empty(true) {}

  template<typename R>
  template<typename A, typename... B, typename>
  Shared<R>::Shared(A&& a, B&&... args)
    : m_state(std::make_shared<Details::SharedState>()),
      m_reactor(std::make_shared<Reactor>(std::forward<A>(a),
        std::forward<B>(args)...)),
      m_is_empty(true) {}

  template<typename R>
  Shared<R>::Shared(Unique<Reactor> reactor)
    : m_state(std::make_shared<Details::SharedState>()),
      m_reactor(std::move(reactor.m_reactor)),
      m_is_empty(true) {}

  template<typename R>
  template<typename U>
  Shared<R>::Shared(Shared<U> reactor)
    : m_state(reactor.m_state),
      m_reactor(std::make_shared<Reactor>(std::move(reactor))),
      m_is_empty(true) {}

  template<typename R>
  Shared<R>::Shared(const Shared& shared)
    : m_state(shared.m_state),
      m_reactor(shared.m_reactor),
      m_is_empty(true) {}

  template<typename R>
  const typename Shared<R>::Reactor& Shared<R>::operator *() const noexcept {
    return *m_reactor;
  }

  template<typename R>
  const typename Shared<R>::Reactor* Shared<R>::operator ->() const noexcept {
    return &*m_reactor;
  }

  template<typename R>
  typename Shared<R>::Reactor& Shared<R>::operator *() noexcept {
    return *m_reactor;
  }

  template<typename R>
  typename Shared<R>::Reactor* Shared<R>::operator ->() noexcept {
    return &*m_reactor;
  }

  template<typename R>
  State Shared<R>::commit(int sequence) noexcept {
    return commit_state(sequence, *m_state, *m_reactor, m_is_empty);
  }

  template<typename R>
  typename Shared<R>::Result Shared<R>::eval() const noexcept(is_noexcept) {
    return m_reactor->eval();
  }

  template<typename R>
  Shared<R>& Shared<R>::operator =(const Shared& shared) {
    m_state = shared.m_state;
    m_reactor = shared.m_reactor;
    m_is_empty = true;
    return *this;
  }

  template<typename R>
  State Shared<R>::commit_state(int sequence, Details::SharedState& state,
      Reactor& reactor, bool& is_empty) {
    if(sequence <= state.m_sequence) {
      if(is_empty && !state.m_is_empty) {
        is_empty = false;
        return combine(state.m_state, State::EVALUATED);
      }
      return state.m_state;
    }
    auto reactor_state = reactor.commit(sequence);
    if(sequence == state.m_sequence) {
      if(has_evaluation(reactor_state)) {
        is_empty = false;
      } else if(is_empty && !state.m_is_empty) {
        is_empty = false;
        reactor_state = combine(reactor_state, State::EVALUATED);
      }
    } else {
      state.m_state = reactor_state;
      state.m_sequence = sequence;
      if(has_evaluation(reactor_state)) {
        state.m_is_empty = false;
        is_empty = false;
      } else if(is_empty && !state.m_is_empty) {
        is_empty = false;
        reactor_state = combine(reactor_state, State::EVALUATED);
      }
    }
    return reactor_state;
  }
}

#endif

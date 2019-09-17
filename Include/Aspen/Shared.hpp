#ifndef ASPEN_SHARED_HPP
#define ASPEN_SHARED_HPP
#include <memory>
#include <utility>
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"
#include "Aspen/Unique.hpp"

namespace Aspen {

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

      /**
       * Constructs a Shared reactor.
       */
      Shared();

      /**
       * Constructs a Shared reactor from a Unique reactor.
       * @param reactor The reactor to transfer ownership from.
       */
      Shared(Unique<Reactor> reactor);

      /**
       * Constructs a Shared reactor.
       * @param args The arguments used to emplace the shared reactor.
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
      explicit Shared(A&& arg, B&&... args);

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
      struct Entry {
        Reactor m_reactor;
        State m_state;
        bool m_is_empty;
        int m_sequence;

        template<typename... A>
        Entry(A&&... args);
      };
      std::shared_ptr<Entry> m_entry;
      bool m_is_empty;
  };

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
  template<typename... A>
  Shared<R>::Entry::Entry(A&&... args)
    : m_reactor(std::forward<A>(args)...),
      m_state(State::NONE),
      m_is_empty(true),
      m_sequence(-1) {}

  template<typename R>
  Shared<R>::Shared()
    : m_entry(std::make_shared<Entry>()),
      m_is_empty(true) {}

  template<typename R>
  Shared<R>::Shared(Unique<Reactor> reactor)
    : m_entry(std::make_shared<Entry>(*reactor.m_reactor)),
      m_is_empty(true) {}

  template<typename R>
  template<typename A, typename>
  Shared<R>::Shared(A&& args)
    : m_entry(std::make_shared<Entry>(std::forward<A>(args))),
      m_is_empty(true) {}

  template<typename R>
  template<typename A, typename... B, typename>
  Shared<R>::Shared(A&& arg, B&&... args)
    : m_entry(std::make_shared<Entry>(std::forward<A>(arg),
        std::forward<B>(args)...)),
      m_is_empty(true) {}

  template<typename R>
  Shared<R>::Shared(const Shared& shared)
    : m_entry(shared.m_entry),
      m_is_empty(true) {}

  template<typename R>
  const typename Shared<R>::Reactor& Shared<R>::operator *() const noexcept {
    return m_entry->m_reactor;
  }

  template<typename R>
  const typename Shared<R>::Reactor* Shared<R>::operator ->() const noexcept {
    return &m_entry->m_reactor;
  }

  template<typename R>
  typename Shared<R>::Reactor& Shared<R>::operator *() noexcept {
    return m_entry->m_reactor;
  }

  template<typename R>
  typename Shared<R>::Reactor* Shared<R>::operator ->() noexcept {
    return &m_entry->m_reactor;
  }

  template<typename R>
  State Shared<R>::commit(int sequence) noexcept {
    if(sequence <= m_entry->m_sequence) {
      auto state = m_entry->m_state;
      if(m_is_empty && !m_entry->m_is_empty) {
        m_is_empty = false;
        state = combine(state, State::EVALUATED);
      }
      return state;
    }
    m_entry->m_state = m_entry->m_reactor.commit(sequence);
    m_entry->m_sequence = sequence;
    auto state = m_entry->m_state;
    if(has_evaluation(state)) {
      m_entry->m_is_empty = false;
      m_is_empty = false;
    } else if(m_is_empty && !m_entry->m_is_empty) {
      m_is_empty = false;
      state = combine(state, State::EVALUATED);
    }
    return state;
  }

  template<typename R>
  typename Shared<R>::Result Shared<R>::eval() const noexcept(is_noexcept) {
    return m_entry->m_reactor.eval();
  }

  template<typename R>
  Shared<R>& Shared<R>::operator =(const Shared& shared) {
    m_entry = shared.m_entry;
    m_is_empty = true;
    return *this;
  }
}

#endif

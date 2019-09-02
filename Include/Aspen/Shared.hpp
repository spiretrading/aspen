#ifndef ASPEN_SHARED_HPP
#define ASPEN_SHARED_HPP
#include <memory>
#include <utility>
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

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
      static constexpr auto is_noexcept = is_noexcept_reactor_v<Reactor>;

      /**
       * Constructs a Shared reactor.
       * @param args The arguments used to emplace the shared reactor.
       */
      template<typename... A>
      explicit Shared(A&&... args);

      /**
       * Constructs a Shared reactor.
       * @param args The arguments used to emplace the shared reactor.
       */
      template<typename A, typename = std::enable_if_t<
        !std::is_base_of_v<Shared, std::decay_t<A>>>>
      explicit Shared(A&& args);

      Shared(const Shared& shared);

      //! Returns a reference to the reactor.
      const Reactor& operator *() const noexcept;

      //! Returns a pointer to the reactor.
      const Reactor* operator ->() const noexcept;

      //! Returns a reference to the reactor.
      Reactor& operator *() noexcept;

      //! Returns a pointer to the reactor.
      Reactor* operator ->() noexcept;

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

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
  template<typename... A>
  Shared<R>::Shared(A&&... args)
    : m_entry(std::make_shared<Entry>(std::forward<A>(args)...)),
      m_is_empty(true) {}

  template<typename R>
  template<typename A, typename>
  Shared<R>::Shared(A&& args)
    : m_entry(std::make_shared<Entry>(std::forward<A>(args))),
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
  eval_result_t<typename Shared<R>::Type> Shared<R>::eval()
      const noexcept(is_noexcept) {
    return m_entry->m_reactor.eval();
  }
}

#endif

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
        int m_previous_sequence;

        template<typename... A>
        Entry(A&&... args);
      };
      std::shared_ptr<Entry> m_entry;
  };

  template<typename A, typename = std::enable_if_t<
    !std::is_base_of_v<Shared<to_reactor_t<A>>, std::decay_t<A>>>>
  Shared(A&&) -> Shared<to_reactor_t<A>>;

  template<typename R>
  template<typename... A>
  Shared<R>::Entry::Entry(A&&... args)
    : m_reactor(std::forward<A>(args)...),
      m_state(State::EMPTY),
      m_previous_sequence(-1) {}

  template<typename R>
  template<typename... A>
  Shared<R>::Shared(A&&... args)
    : m_entry(std::make_shared<Entry>(std::forward<A>(args)...)) {}

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
    if(sequence == m_entry->m_previous_sequence ||
        is_complete(m_entry->m_state)) {
      return m_entry->m_state;
    }
    m_entry->m_state = m_entry->m_reactor.commit(sequence);
    m_entry->m_previous_sequence = sequence;
    return m_entry->m_state;
  }

  template<typename R>
  eval_result_t<typename Shared<R>::Type> Shared<R>::eval()
      const noexcept(is_noexcept) {
    return m_entry->m_reactor.eval();
  }
}

#endif

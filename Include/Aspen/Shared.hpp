#ifndef ASPEN_SHARED_HPP
#define ASPEN_SHARED_HPP
#include <concepts>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include "Aspen/Box.hpp"
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"
#include "Aspen/Unique.hpp"

namespace Aspen {
namespace Details {
  struct Sequence {
    std::uint64_t m_value;
    bool m_is_set;

    Sequence() noexcept;

    void set(std::uint64_t value) noexcept;
    bool is_at(std::uint64_t value) const noexcept;
  };

  bool operator <(Sequence left, Sequence right) noexcept;

  struct SharedState {
    State m_state;
    Sequence m_sequence;
    Sequence m_last_evaluation;
    CommitFlag m_flag;

    SharedState() noexcept;
  };

  template<typename R>
  struct SharedEvaluator {
    std::shared_ptr<SharedState> m_state;
    std::weak_ptr<R> m_reactor;
    Sequence m_sequence;
    std::optional<try_maybe_t<reactor_result_t<R>, !is_noexcept_reactor_v<R>>>
      m_evaluation;

    explicit SharedEvaluator(std::shared_ptr<SharedState> state) noexcept;
  };

  inline Sequence::Sequence() noexcept
    : m_value(0),
      m_is_set(false) {}

  inline void Sequence::set(std::uint64_t value) noexcept {
    m_value = value;
    m_is_set = true;
  }

  inline bool Sequence::is_at(std::uint64_t value) const noexcept {
    return m_is_set && m_value == value;
  }

  inline bool operator <(Sequence left, Sequence right) noexcept {
    return right.m_is_set && (!left.m_is_set || left.m_value < right.m_value);
  }

  inline SharedState::SharedState() noexcept
    : m_state(State::NONE) {}

  template<typename R>
  SharedEvaluator<R>::SharedEvaluator(
    std::shared_ptr<SharedState> state) noexcept
    : m_state(std::move(state)) {}
}

  /**
   * Used to share a reactor as a child among multiple reactors.
   * @param <R> The type of reactor being shared.
   */
  template<IsReactor R>
  class Shared {
    public:

      /** The type of reactor being shared. */
      using Reactor = R;

      /** The type to evaluate to. */
      using Type = reactor_result_t<Reactor>;

      /** The type returned by an evaluation. */
      using Result = decltype(std::declval<Reactor&>().eval());

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_reactor_v<Reactor>;

      /** Constructs a Shared reactor. */
      Shared();

      /**
       * Constructs a Shared reactor.
       * @param args The arguments used to emplace the shared reactor.
       */
      template<typename A, typename... B> requires(
        !std::derived_from<std::remove_cvref_t<A>, Shared<R>>)
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

      Shared(const Shared& shared) noexcept;
      Shared(Shared&& shared) noexcept;
      ~Shared();

      const Reactor& operator *() const noexcept;
      const Reactor* operator ->() const noexcept;
      Reactor& operator *() noexcept;
      Reactor* operator ->() noexcept;
      State commit(std::uint64_t sequence) noexcept;
      Result eval() const noexcept(is_noexcept);
      Shared& operator =(const Shared& shared) noexcept;
      Shared& operator =(Shared&& shared) noexcept;

    private:
      template<IsReactor> friend class Shared;
      template<IsReactor> friend class Weak;
      std::shared_ptr<Details::SharedEvaluator<Reactor>> m_evaluator;
      std::shared_ptr<Reactor> m_reactor;
      Details::Sequence m_last_evaluation;
      CommitFlag* m_parent;

      static State commit_state(std::uint64_t sequence, Reactor& reactor,
        Details::SharedEvaluator<Reactor>& evaluator,
        Details::Sequence& last_evaluation, CommitFlag* current);

      Shared(std::shared_ptr<Details::SharedEvaluator<Reactor>> evaluator,
        std::shared_ptr<Reactor> reactor) noexcept;
      void release() noexcept;
      void set_parent(CommitFlag* parent) noexcept;
  };

  /** Type alias for a Shared<Box<T>>. */
  template<typename T>
  using SharedBox = Shared<Box<T>>;

  /**
   * Boxes a reactor into a copyable generic interface.
   * @param reactor The reactor to wrap.
   * @return A SharedBox wrapping the <i>reactor</i>.
   */
  template<typename R> requires IsReactor<to_reactor_t<R>>
  auto shared_box(R&& reactor) {
    return SharedBox<reactor_result_t<R>>(std::forward<R>(reactor));
  }

  /**
   * A type trait that wraps a type in a Shared, collapsing any Shared it is
   * already wrapped in.
   * @param <T> The type to wrap.
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

  template<typename A> requires(
    !std::derived_from<std::remove_cvref_t<A>, Shared<to_reactor_t<A>>>)
  Shared(A&&) -> Shared<to_reactor_t<A>>;

  template<IsReactor R>
  Shared<R>::Shared()
    : Shared(std::make_shared<Details::SharedEvaluator<Reactor>>(
        std::make_shared<Details::SharedState>()),
        std::make_shared<Reactor>()) {
    m_evaluator->m_reactor = m_reactor;
  }

  template<IsReactor R>
  template<typename A, typename... B> requires(
    !std::derived_from<std::remove_cvref_t<A>, Shared<R>>)
  Shared<R>::Shared(A&& a, B&&... args)
    : Shared(std::make_shared<Details::SharedEvaluator<Reactor>>(
        std::make_shared<Details::SharedState>()),
        std::make_shared<Reactor>(std::forward<A>(a),
        std::forward<B>(args)...)) {
    m_evaluator->m_reactor = m_reactor;
  }

  template<IsReactor R>
  Shared<R>::Shared(Unique<Reactor> reactor)
    : Shared(std::make_shared<Details::SharedEvaluator<Reactor>>(
        std::make_shared<Details::SharedState>()),
        std::shared_ptr<Reactor>(std::move(reactor.m_reactor))) {
    m_evaluator->m_reactor = m_reactor;
  }

  template<IsReactor R>
  template<typename U>
  Shared<R>::Shared(Shared<U> reactor)
    : Shared(std::make_shared<Details::SharedEvaluator<Reactor>>(
        reactor.m_evaluator->m_state), std::make_shared<Reactor>(reactor)) {
    m_evaluator->m_reactor = m_reactor;
  }

  template<IsReactor R>
  Shared<R>::Shared(const Shared& shared) noexcept
    : Shared(shared.m_evaluator, shared.m_reactor) {}

  template<IsReactor R>
  Shared<R>::Shared(Shared&& shared) noexcept
      : m_evaluator(std::move(shared.m_evaluator)),
        m_reactor(std::move(shared.m_reactor)),
        m_last_evaluation(shared.m_last_evaluation),
        m_parent(nullptr) {
    if(shared.m_parent) {
      m_evaluator->m_state->m_flag.remove_parent(*shared.m_parent);
      shared.m_parent = nullptr;
    }
  }

  template<IsReactor R>
  Shared<R>::~Shared() {
    if(m_evaluator) {
      release();
    }
  }

  template<IsReactor R>
  const typename Shared<R>::Reactor& Shared<R>::operator *() const noexcept {
    return *m_reactor;
  }

  template<IsReactor R>
  const typename Shared<R>::Reactor* Shared<R>::operator ->() const noexcept {
    return &*m_reactor;
  }

  template<IsReactor R>
  typename Shared<R>::Reactor& Shared<R>::operator *() noexcept {
    return *m_reactor;
  }

  template<IsReactor R>
  typename Shared<R>::Reactor* Shared<R>::operator ->() noexcept {
    return &*m_reactor;
  }

  template<IsReactor R>
  State Shared<R>::commit(std::uint64_t sequence) noexcept {
    auto current = CommitFlag::get_current();
    auto state = commit_state(
      sequence, *m_reactor, *m_evaluator, m_last_evaluation, current);
    if(current != m_parent) {
      set_parent(current);
    }
    return state;
  }

  template<IsReactor R>
  typename Shared<R>::Result Shared<R>::eval() const noexcept(is_noexcept) {
    return m_reactor->eval();
  }

  template<IsReactor R>
  Shared<R>& Shared<R>::operator =(const Shared& shared) noexcept {
    if(this == &shared) {
      return *this;
    }
    if(m_evaluator) {
      release();
    }
    m_evaluator = shared.m_evaluator;
    m_reactor = shared.m_reactor;
    m_last_evaluation = Details::Sequence();
    return *this;
  }

  template<IsReactor R>
  Shared<R>& Shared<R>::operator =(Shared&& shared) noexcept {
    if(this == &shared) {
      return *this;
    }
    if(m_evaluator) {
      release();
    }
    m_evaluator = std::move(shared.m_evaluator);
    m_reactor = std::move(shared.m_reactor);
    m_last_evaluation = shared.m_last_evaluation;
    m_parent = nullptr;
    if(shared.m_parent) {
      m_evaluator->m_state->m_flag.remove_parent(*shared.m_parent);
      shared.m_parent = nullptr;
    }
    return *this;
  }

  template<IsReactor R>
  State Shared<R>::commit_state(std::uint64_t sequence, Reactor& reactor,
      Details::SharedEvaluator<Reactor>& evaluator,
      Details::Sequence& last_evaluation, CommitFlag* current) {
    if(evaluator.m_sequence.m_is_set &&
        sequence <= evaluator.m_sequence.m_value) {
      if(last_evaluation < evaluator.m_state->m_last_evaluation) {
        last_evaluation = evaluator.m_state->m_last_evaluation;
        return combine(evaluator.m_state->m_state, State::EVALUATED);
      }
      return evaluator.m_state->m_state;
    }
    auto& flag = evaluator.m_state->m_flag;
    if(current != &flag && !flag.is_raised() && evaluator.m_sequence.m_is_set) {
      auto skipped = reset(
        evaluator.m_state->m_state, combine(State::EVALUATED, State::CONTINUE));
      evaluator.m_state->m_state = skipped;
      evaluator.m_state->m_sequence.set(sequence);
      evaluator.m_sequence.set(sequence);
      if(last_evaluation < evaluator.m_state->m_last_evaluation) {
        last_evaluation = evaluator.m_state->m_last_evaluation;
        return combine(skipped, State::EVALUATED);
      }
      return skipped;
    }
    flag.clear();
    auto reactor_state = [&] {
      auto scope = CommitFlagScope(flag);
      return reactor.commit(sequence);
    }();
    if(has_continuation(reactor_state)) {
      flag.raise();
    }
    evaluator.m_sequence.set(sequence);
    if(evaluator.m_state->m_sequence.is_at(sequence)) {
      if(last_evaluation < evaluator.m_state->m_last_evaluation) {
        last_evaluation = evaluator.m_state->m_last_evaluation;
        reactor_state = combine(reactor_state, State::EVALUATED);
      }
    } else {
      evaluator.m_state->m_state = reactor_state;
      evaluator.m_state->m_sequence.set(sequence);
      if(has_evaluation(reactor_state)) {
        evaluator.m_state->m_last_evaluation.set(sequence);
        last_evaluation.set(sequence);
      } else if(last_evaluation < evaluator.m_state->m_last_evaluation) {
        last_evaluation = evaluator.m_state->m_last_evaluation;
        reactor_state = combine(reactor_state, State::EVALUATED);
      }
    }
    return reactor_state;
  }

  template<IsReactor R>
  Shared<R>::Shared(
    std::shared_ptr<Details::SharedEvaluator<Reactor>> evaluator,
    std::shared_ptr<Reactor> reactor) noexcept
    : m_evaluator(std::move(evaluator)),
      m_reactor(std::move(reactor)),
      m_parent(nullptr) {}

  template<IsReactor R>
  void Shared<R>::release() noexcept {
    set_parent(nullptr);
    if(m_evaluator.use_count() > 1 && m_reactor.use_count() == 1 &&
        m_evaluator->m_state->m_last_evaluation.m_is_set) {
      try_assign(m_evaluator->m_evaluation, *m_reactor);
    }
  }

  template<IsReactor R>
  void Shared<R>::set_parent(CommitFlag* parent) noexcept {
    auto& flag = m_evaluator->m_state->m_flag;
    if(parent == &flag) {
      parent = nullptr;
    }
    if(parent == m_parent) {
      return;
    }
    if(m_parent) {
      flag.remove_parent(*m_parent);
    }
    m_parent = parent;
    if(m_parent) {
      flag.add_parent(*m_parent);
    }
  }
}

#endif

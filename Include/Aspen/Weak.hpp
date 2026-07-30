#ifndef ASPEN_WEAK_HPP
#define ASPEN_WEAK_HPP
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/Shared.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a weak reference to an existing shared reactor.
   * @param <R> The type of reactor to observe.
   */
  template<IsReactor R>
  class Weak {
    public:

      /** The type of reactor being observed. */
      using Reactor = R;

      /** The type to evaluate to. */
      using Type = reactor_result_t<Reactor>;

      /** The type returned by an evaluation. */
      using Result = decltype(std::declval<Reactor&>().eval());

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_reactor_v<Reactor>;

      /**
       * Constructs a Weak reactor observing an existing Shared reactor.
       * @param reactor The reactor to observe.
       */
      explicit Weak(const Shared<Reactor>& reactor) noexcept;

      Weak(const Weak& weak) noexcept;
      Weak(Weak&& weak) noexcept;
      ~Weak();

      /** Returns a new Shared reactor to the reactor being observed. */
      std::optional<Shared<Reactor>> lock() const noexcept;

      State commit(std::uint64_t sequence) noexcept;
      Result eval() const noexcept(is_noexcept);

    private:
      std::shared_ptr<Details::SharedEvaluator<Reactor>> m_evaluator;
      Details::Sequence m_last_evaluation;
      CommitFlag* m_parent;

      void set_parent(CommitFlag* parent) noexcept;
  };

  template<IsReactor R>
  Weak<R>::Weak(const Shared<Reactor>& reactor) noexcept
    : m_evaluator(reactor.m_evaluator),
      m_parent(nullptr) {}

  template<IsReactor R>
  Weak<R>::Weak(const Weak& weak) noexcept
    : m_evaluator(weak.m_evaluator),
      m_parent(nullptr) {}

  template<IsReactor R>
  Weak<R>::Weak(Weak&& weak) noexcept
      : m_evaluator(std::move(weak.m_evaluator)),
        m_last_evaluation(weak.m_last_evaluation),
        m_parent(weak.m_parent) {
    weak.m_parent = nullptr;
  }

  template<IsReactor R>
  Weak<R>::~Weak() {
    if(m_evaluator) {
      set_parent(nullptr);
    }
  }

  template<IsReactor R>
  std::optional<Shared<R>> Weak<R>::lock() const noexcept {
    auto reactor = m_evaluator->m_reactor.lock();
    if(!reactor) {
      return std::nullopt;
    }
    return Shared(m_evaluator, std::move(reactor));
  }

  template<IsReactor R>
  State Weak<R>::commit(std::uint64_t sequence) noexcept {
    auto reactor = m_evaluator->m_reactor.lock();
    if(!reactor) {
      if(m_last_evaluation < m_evaluator->m_state->m_last_evaluation) {
        return State::COMPLETE_EVALUATED;
      }
      return State::COMPLETE;
    }
    auto current = CommitFlag::get_current();
    auto state = Shared<Reactor>::commit_state(
      sequence, *reactor, *m_evaluator, m_last_evaluation, current);
    if(current != m_parent) {
      set_parent(current);
    }
    return state;
  }

  template<IsReactor R>
  typename Weak<R>::Result Weak<R>::eval() const noexcept(is_noexcept) {
    if(m_evaluator->m_evaluation) {
      return **m_evaluator->m_evaluation;
    }
    auto reactor = m_evaluator->m_reactor.lock();
    return reactor->eval();
  }

  template<IsReactor R>
  void Weak<R>::set_parent(CommitFlag* parent) noexcept {
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

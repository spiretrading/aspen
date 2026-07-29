#ifndef ASPEN_CONCUR_HPP
#define ASPEN_CONCUR_HPP
#include <concepts>
#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include "Aspen/Branch.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to every value produced by its
   * children.
   * @param <T> The type of reactor producing the reactors to evaluate to.
   */
  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  class Concur {
    public:

      /** The type to evaluate to. */
      using Type = reactor_result_t<reactor_result_t<T>>;

      /** Whether an evaluation is noexcept. */
      static constexpr auto is_noexcept =
        is_noexcept_reactor_v<reactor_result_t<T>>;

      /**
       * Constructs a Concur.
       * @param producer The reactor producing the reactors to evaluate to.
       */
      template<typename TF> requires std::constructible_from<T, TF>
      explicit Concur(TF&& producer);

      State commit(std::uint64_t sequence) noexcept;
      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      struct Child {
        Branch<reactor_result_t<T>> m_reactor;
        bool m_is_complete;

        template<typename U> requires
          std::constructible_from<reactor_result_t<T>, U>
        explicit Child(U&& reactor);
      };
      std::optional<Branch<T>> m_producer;
      std::unique_ptr<std::list<Child>> m_children;
      typename std::list<Child>::iterator m_current;
      typename std::list<Child>::iterator m_position;

      void increment();
  };

  template<typename T> requires(
    !std::derived_from<std::remove_cvref_t<T>, Concur<to_reactor_t<T>>>)
  Concur(T&&) -> Concur<to_reactor_t<T>>;

  /**
   * Builds a Concur reactor to evaluate the reactors produced by its child.
   * @param producer The reactor producing the reactors to evaluate to.
   * @return A reactor evaluating to every value its children produce.
   */
  template<typename T> requires IsReactor<std::remove_cvref_t<T>>
  auto concur(T&& producer) {
    return Concur(std::forward<T>(producer));
  }

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  template<typename U> requires std::constructible_from<reactor_result_t<T>, U>
  Concur<T>::Child::Child(U&& reactor)
    : m_reactor(std::forward<U>(reactor)),
      m_is_complete(false) {}

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  template<typename TF> requires std::constructible_from<T, TF>
  Concur<T>::Concur(TF&& producer)
    : m_producer(std::forward<TF>(producer)),
      m_children(std::make_unique<std::list<Child>>()),
      m_current(m_children->end()),
      m_position(m_children->end()) {}

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  State Concur<T>::commit(std::uint64_t sequence) noexcept {
    auto state = [&] {
      if(m_producer) {
        auto producer_state = m_producer->commit(sequence);
        if(has_evaluation(producer_state)) {
          try {
            m_children->emplace_back((*m_producer)->eval());
            if(m_children->size() == 1) {
              m_position = m_children->begin();
            }
          } catch(...) {}
        }
        if(has_continuation(producer_state)) {
          return State::CONTINUE;
        }
        if(is_complete(producer_state)) {
          m_producer = std::nullopt;
        }
      }
      return State::NONE;
    }();
    while(m_position != m_current && m_position->m_is_complete) {
      m_position = m_children->erase(m_position);
      if(m_position == m_children->end()) {
        m_position = m_children->begin();
      }
    }
    if(!m_children->empty()) {
      auto start = m_position;
      while(true) {
        auto& child = *m_position;
        if(child.m_is_complete) {
          if(m_position != m_current) {
            m_position = m_children->erase(m_position);
            if(m_position == m_children->end()) {
              m_position = m_children->begin();
            }
          } else {
            increment();
          }
        } else {
          auto child_state = child.m_reactor.commit(sequence);
          if(has_continuation(child_state)) {
            state = combine(state, State::CONTINUE);
          } else if(is_complete(child_state)) {
            child.m_is_complete = true;
          }
          if(has_evaluation(child_state)) {
            state = combine(state, State::EVALUATED);
            if(m_children->size() > 1) {
              state = combine(state, State::CONTINUE);
            }
            m_current = m_position;
            increment();
            break;
          } else {
            increment();
          }
        }
        if(m_position == start) {
          break;
        }
      }
    }
    if((m_children->empty() ||
        m_children->size() == 1 && m_children->front().m_is_complete) &&
          !m_producer) {
      state = combine(state, State::COMPLETE);
    }
    return state;
  }

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  eval_result_t<typename Concur<T>::Type> Concur<T>::eval()
      const noexcept(is_noexcept) {
    return m_current->m_reactor->eval();
  }

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  void Concur<T>::increment() {
    ++m_position;
    if(m_position == m_children->end()) {
      m_position = m_children->begin();
    }
  }
}

#endif

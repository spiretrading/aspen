#ifndef ASPEN_CONCUR_HPP
#define ASPEN_CONCUR_HPP
#include <algorithm>
#include <atomic>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
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

      /** The type returned by an evaluation. */
      using Result = common_evaluation_t<reactor_result_t<T>>;

      /** Whether an evaluation is noexcept. */
      static constexpr auto is_noexcept =
        is_noexcept_evaluation_v<reactor_result_t<T>>;

      /**
       * Constructs a Concur.
       * @param producer The reactor producing the reactors to evaluate to.
       */
      template<typename TF> requires std::constructible_from<T, TF>
      explicit Concur(TF&& producer);

      State commit(std::uint64_t sequence) noexcept;
      Result eval() const noexcept(is_noexcept);

    private:
      static constexpr auto BITS = std::size_t(64);
      static constexpr auto NO_CHILD = std::size_t(-1);
      struct Child {
        Branch<reactor_result_t<T>> m_reactor;
        bool m_is_complete;

        template<typename U> requires
          std::constructible_from<reactor_result_t<T>, U>
        explicit Child(U&& reactor);
      };
      std::optional<Branch<T>> m_producer;
      std::deque<std::optional<Child>> m_children;
      std::deque<std::atomic_uint64_t> m_raised;
      std::vector<std::size_t> m_free;
      std::size_t m_count;
      std::size_t m_current;
      std::size_t m_position;

      void add(auto&& reactor);
      void remove(std::size_t index) noexcept;
      void raise_slot(std::size_t index) noexcept;
      void clear_slot(std::size_t index) noexcept;
      std::size_t next(std::size_t index) const noexcept;
      std::size_t find_raised(
        std::size_t from, std::size_t count) const noexcept;
  };

  template<typename T> requires(
    !std::derived_from<std::remove_cvref_t<T>, Concur<to_reactor_t<T>>>)
  Concur(T&&) -> Concur<to_reactor_t<T>>;

  /**
   * Builds a Concur reactor to evaluate the reactors produced by its child.
   * @param producer The reactor producing the reactors to evaluate to.
   * @return A reactor evaluating to every value its children produce.
   */
  template<typename T> requires IsReactor<std::remove_cvref_t<T>> &&
    IsReactor<reactor_result_t<T>>
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
      m_count(0),
      m_current(NO_CHILD),
      m_position(0) {}

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  State Concur<T>::commit(std::uint64_t sequence) noexcept {
    auto state = [&] {
      if(m_producer) {
        auto producer_state = m_producer->commit(sequence);
        if(has_evaluation(producer_state)) {
          try {
            add((*m_producer)->eval());
          } catch(...) {}
        }
        if(is_complete(producer_state)) {
          m_producer = std::nullopt;
        } else if(has_continuation(producer_state)) {
          return State::CONTINUE;
        }
      }
      return State::NONE;
    }();
    auto start = m_position;
    auto has_evaluated = false;
    auto remaining = m_count == 0 ? std::size_t(0) : m_children.size();
    while(remaining != 0) {
      auto index = find_raised(m_position, remaining);
      if(index == NO_CHILD) {
        break;
      }
      remaining -= (index + m_children.size() - m_position) % m_children.size();
      auto& child = *m_children[index];
      clear_slot(index);
      if(child.m_is_complete) {
        if(index != m_current) {
          remove(index);
        }
      } else {
        auto child_state = child.m_reactor.commit(sequence);
        if(is_complete(child_state)) {
          child.m_is_complete = true;
          if(has_evaluation(child_state) || index == m_current) {
            raise_slot(index);
          } else {
            remove(index);
          }
        } else if(has_continuation(child_state)) {
          state = combine(state, State::CONTINUE);
        }
        if(has_evaluation(child_state)) {
          state = combine(state, State::EVALUATED);
          if(m_count > 1) {
            state = combine(state, State::CONTINUE);
          }
          if(m_current != index && m_current != NO_CHILD &&
              m_children[m_current] && m_children[m_current]->m_is_complete) {
            remove(m_current);
          }
          m_current = index;
          m_position = next(index);
          has_evaluated = true;
          break;
        }
      }
      m_position = next(index);
      --remaining;
    }
    if(!has_evaluated) {
      m_position = start;
    }
    if((m_count == 0 || m_count == 1 && m_current != NO_CHILD &&
        m_children[m_current] && m_children[m_current]->m_is_complete) &&
        !m_producer) {
      state = combine(reset(state, State::CONTINUE), State::COMPLETE);
    }
    return state;
  }

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  typename Concur<T>::Result Concur<T>::eval() const noexcept(is_noexcept) {
    return m_children[m_current]->m_reactor->eval();
  }

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  void Concur<T>::add(auto&& reactor) {
    auto index = std::size_t(0);
    if(m_free.empty()) {
      index = m_children.size();
      if(index % BITS == 0) {
        m_raised.emplace_back(0);
      }
      if(m_free.capacity() < index + 1) {
        m_free.reserve(std::max(2 * (index + 1), std::size_t(4)));
      }
      m_children.emplace_back(
        std::in_place, std::forward<decltype(reactor)>(reactor));
    } else {
      index = m_free.back();
      m_children[index].emplace(std::forward<decltype(reactor)>(reactor));
      m_free.pop_back();
    }
    m_children[index]->m_reactor.set_slot(
      &m_raised[index / BITS], static_cast<std::uint8_t>(index % BITS));
    if(m_count == 0) {
      m_position = index;
    }
    ++m_count;
  }

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  void Concur<T>::remove(std::size_t index) noexcept {
    clear_slot(index);
    m_children[index].reset();
    m_free.push_back(index);
    --m_count;
  }

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  void Concur<T>::raise_slot(std::size_t index) noexcept {
    m_raised[index / BITS].fetch_or(
      std::uint64_t(1) << (index % BITS), std::memory_order_acq_rel);
  }

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  void Concur<T>::clear_slot(std::size_t index) noexcept {
    m_raised[index / BITS].fetch_and(
      ~(std::uint64_t(1) << (index % BITS)), std::memory_order_acq_rel);
  }

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  std::size_t Concur<T>::next(std::size_t index) const noexcept {
    if(index + 1 == m_children.size()) {
      return 0;
    }
    return index + 1;
  }

  template<IsReactor T> requires IsReactor<reactor_result_t<T>>
  std::size_t Concur<T>::find_raised(
      std::size_t from, std::size_t count) const noexcept {
    auto slots = m_children.size();
    auto i = std::size_t(0);
    while(i < count) {
      auto index = (from + i) % slots;
      auto bit = index % BITS;
      auto span = std::min(BITS - bit, slots - index);
      auto bits = m_raised[index / BITS].load(std::memory_order_acquire) >> bit;
      if(span != BITS - bit) {
        bits &= (std::uint64_t(1) << span) - 1;
      }
      while(bits != 0) {
        auto offset = static_cast<std::size_t>(std::countr_zero(bits));
        if(i + offset >= count) {
          return NO_CHILD;
        }
        if(m_children[index + offset]) {
          return index + offset;
        }
        bits &= bits - 1;
      }
      i += span;
    }
    return NO_CHILD;
  }
}

#endif

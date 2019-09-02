#ifndef ASPEN_CONCAT_HPP
#define ASPEN_CONCAT_HPP
#include <list>
#include <type_traits>
#include <utility>
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to every value produced by its
   * children.
   * @param <R> The type of reactor producing the reactors to evaluate to.
   */
  template<typename R>
  class Concat {
    public:
      using Reactor = R;
      using Type = reactor_result_t<reactor_result_t<Reactor>>;
      static constexpr auto is_noexcept =
        is_noexcept_reactor_v<reactor_result_t<Reactor>>;

      /**
       * Constructs a Concat.
       * @param producer The reactor producing the reactors to evaluate to.
       */
      template<typename RF, typename = std::enable_if_t<
        !std::is_base_of_v<Concat, std::decay_t<RF>>>>
      explicit Concat(RF&& producer);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      Reactor m_producer;
      State m_producer_state;
      std::list<reactor_result_t<Reactor>> m_children;
      State m_child_state;
      State m_state;
      int m_previous_sequence;
  };

  template<typename R, typename = std::enable_if_t<
    !std::is_base_of_v<Concat<to_reactor_t<R>>, std::decay_t<R>>>>
  Concat(R&&) -> Concat<to_reactor_t<R>>;

  /**
   * Concats the reactors produced by its child.
   * @param producer The reactor producing the reactors to evaluate to.
   */
  template<typename R>
  auto concat(R&& producer) {
    return Concat(std::forward<R>(producer));
  }

  template<typename R>
  template<typename RF, typename>
  Concat<R>::Concat(RF&& producer)
    : m_producer(std::forward<RF>(producer)),
      m_producer_state(State::NONE),
      m_child_state(State::NONE) {}

  template<typename R>
  State Concat<R>::commit(int sequence) noexcept {
    auto state = State::NONE;
    if(!is_complete(m_producer_state)) {
      m_producer_state = m_producer.commit(sequence);
      if(has_evaluation(m_producer_state)) {
        try {
          m_children.emplace_back(m_producer.eval());
        } catch(const std::exception&) {}
      }
      if(has_continuation(m_producer_state)) {
        state = combine(state, State::CONTINUE);
      } else if(is_complete(m_producer_state) &&
          (m_children.empty() ||
          m_children.size() == 1 && is_complete(m_child_state))) {
        state = combine(state, State::COMPLETE);
      }
    }
    auto child_state = [&] {
      if(is_complete(m_child_state)) {
        while(m_children.size() > 1) {
          auto child_state = std::next(m_children.begin())->commit(sequence);
          if(has_evaluation(child_state)) {
            m_children.pop_front();
            return child_state;
          } else if(is_complete(child_state)) {
            m_children.erase(std::next(m_children.begin()));
          } else {
            break;
          }
        }
      }
      if(!m_children.empty() && !is_complete(m_child_state)) {
        return m_children.front().commit(sequence);
      }
      return State::COMPLETE;
    }();
    if(has_evaluation(child_state)) {
      state = combine(state, State::EVALUATED);
      if(is_complete(child_state) && m_children.size() > 1) {
        state = combine(state, State::CONTINUE);
      }
    } else if(is_complete(child_state)) {
      while(m_children.size() > 1) {
        auto next_child_state = std::next(m_children.begin())->commit(sequence);
        if(!is_empty(next_child_state)) {
          m_children.pop_front();
          state = combine(state, State::EVALUATED);
          if(is_complete(next_child_state) && m_children.size() > 1) {
            state = combine(state, State::CONTINUE);
          }
          child_state = next_child_state;
          break;
        } else if(is_complete(next_child_state)) {
          m_children.erase(std::next(m_children.begin()));
        } else {
          break;
        }
      }
    }
    if(has_continuation(child_state)) {
      state = combine(state, State::CONTINUE);
    } else if(is_complete(child_state) &&
        m_children.size() == 1 && is_complete(m_producer_state)) {
      state = combine(state, State::COMPLETE);
    }
    m_child_state = child_state;
    return state;
  }

  template<typename T>
  eval_result_t<typename Concat<T>::Type> Concat<T>::eval()
      const noexcept(is_noexcept) {
    return m_children.front().eval();
  }
}

#endif

module;
#include <list>
#include <type_traits>
#include <utility>

export module Aspen:Concat;

import :State;
import :Traits;

export namespace Aspen {

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
      bool m_is_producer_complete;
      State m_producer_state;
      std::list<reactor_result_t<Reactor>> m_children;
      bool m_is_child_complete;
  };

  template<typename R, typename = std::enable_if_t<
    !std::is_base_of_v<Concat<to_reactor_t<R>>, std::decay_t<R>>>>
  Concat(R&&) -> Concat<to_reactor_t<R>>;

  /**
   * Concats the reactors produced by its child.
   * @param producer The reactor producing the reactors to evaluate to.
   */
  template<typename R, std::enable_if_t<is_reactor_v<R>>* = nullptr>
  auto concat(R&& producer) {
    return Concat(std::forward<R>(producer));
  }

  template<typename R>
  template<typename RF, typename>
  Concat<R>::Concat(RF&& producer)
    : m_producer(std::forward<RF>(producer)),
      m_is_producer_complete(false),
      m_is_child_complete(false) {}

  template<typename R>
  State Concat<R>::commit(int sequence) noexcept {
    auto state = [&] {
      if(!m_is_producer_complete) {
        auto producer_state = m_producer.commit(sequence);
        if(has_evaluation(producer_state)) {
          try {
            m_children.emplace_back(m_producer.eval());
          } catch(...) {}
        }
        if(has_continuation(producer_state)) {
          return State::CONTINUE;
        }
        m_is_producer_complete = is_complete(producer_state);
        if(m_is_producer_complete && (m_children.empty() ||
            m_children.size() == 1 && m_is_child_complete)) {
          return State::COMPLETE;
        }
      }
      return State::NONE;
    }();
    auto child_state = [&] {
      while(true) {
        if(m_is_child_complete) {
          while(m_children.size() > 1) {
            auto next_child = std::next(m_children.begin());
            auto child_state = next_child->commit(sequence);
            if(has_evaluation(child_state)) {
              m_is_child_complete = is_complete(child_state);
              m_children.pop_front();
              return child_state;
            } else if(is_complete(child_state)) {
              m_children.erase(next_child);
            } else if(has_continuation(child_state)) {
              return State::CONTINUE;
            } else {
              break;
            }
          }
          return State::NONE;
        } else if(!m_children.empty()) {
          auto child_state = m_children.front().commit(sequence);
          m_is_child_complete = is_complete(child_state);
          if(!m_is_child_complete || has_evaluation(child_state)) {
            return child_state;
          }
        } else {
          return State::NONE;
        }
      }
    }();
    if(has_evaluation(child_state)) {
      state = combine(state, State::EVALUATED);
      if(m_is_child_complete && m_children.size() > 1) {
        state = combine(state, State::CONTINUE);
      }
    }
    if(has_continuation(child_state)) {
      state = combine(state, State::CONTINUE);
    } else if(m_is_child_complete && m_children.size() == 1 &&
        m_is_producer_complete) {
      state = combine(state, State::COMPLETE);
    }
    return state;
  }

  template<typename T>
  eval_result_t<typename Concat<T>::Type> Concat<T>::eval()
      const noexcept(is_noexcept) {
    return m_children.front().eval();
  }
}

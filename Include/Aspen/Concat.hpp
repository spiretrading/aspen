#ifndef ASPEN_CONCAT_HPP
#define ASPEN_CONCAT_HPP
#include <deque>
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor evaluates to every value produces by its children.
   * @param <T> The type of reactor producing the reactors to evaluate to.
   */
  template<typename T>
  class Concat {
    public:
      using Type = reactor_result_t<reactor_result_t<T>>;

      static constexpr auto is_noexcept =
        is_noexcept_reactor_v<reactor_result_t<T>>;

      /**
       * Constructs a Concat.
       * @param producer The reactor producing the reactors to evaluate to.
       */
      template<typename TF>
      Concat(TF&& producer);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      T m_producer;
      State m_producer_state;
      std::deque<reactor_result_t<T>> m_children;
      State m_child_state;
      State m_state;
      int m_previous_sequence;
  };

  template<typename T>
  Concat(T&&) -> Concat<to_reactor_t<T>>;

  /**
   * Concats the reactors produced by its child.
   * @param producer The reactor producing the reactors to evaluate to.
   */
  template<typename T>
  auto concat(T&& producer) {
    return Concat(std::forward<T>(producer));
  }

  template<typename T>
  template<typename TF>
  Concat<T>::Concat(TF&& producer)
    : m_producer(std::forward<TF>(producer)),
      m_producer_state(State::EMPTY),
      m_child_state(State::EMPTY),
      m_state(State::EMPTY),
      m_previous_sequence(-1) {}

  template<typename T>
  State Concat<T>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    if(is_empty(m_state)) {
      m_state = State::EMPTY;
    } else {
      m_state = State::NONE;
    }
    if(!is_complete(m_producer_state)) {
      auto producer_state = Aspen::commit(m_producer, sequence);
      if(has_evaluation(producer_state) ||
          is_empty(m_producer_state) && !is_empty(producer_state)) {
        try {
          m_children.emplace_back(Aspen::eval(m_producer));
        } catch(const std::exception&) {}
      }
      m_producer_state = producer_state;
      if(has_continuation(producer_state)) {
        m_state = combine(m_state, State::CONTINUE);
      } else if(is_complete(producer_state)) {
        if(m_children.empty() ||
            m_children.size() == 1 && is_complete(m_child_state)) {
          m_state = combine(m_state, State::COMPLETE);
        }
      }
    }
    if(is_complete(m_child_state) && m_children.size() > 1) {
      m_children.pop_front();
      m_child_state = State::EMPTY;
    }
    if(!m_children.empty() && !is_complete(m_child_state)) {
      auto child_state = Aspen::commit(m_children.front(), sequence);
      if(has_evaluation(child_state) ||
          is_empty(m_child_state) && !is_empty(child_state)) {
        m_state = combine(m_state, State::EVALUATED);
      }
      if(has_continuation(child_state) ||
          is_complete(child_state) && m_children.size() > 1) {
        m_state = combine(m_state, State::CONTINUE);
      } else if(is_complete(child_state)) {
        if(m_children.size() == 1 && is_complete(m_producer_state)) {
          m_state = combine(m_state, State::COMPLETE);
        }
      }
      m_child_state = child_state;
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename T>
  eval_result_t<typename Concat<T>::Type> Concat<T>::eval()
      const noexcept(is_noexcept) {
    return Aspen::eval(m_children.front());
  }
}

#endif

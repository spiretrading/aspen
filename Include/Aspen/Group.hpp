#ifndef ASPEN_GROUP_HPP
#define ASPEN_GROUP_HPP
#include <list>
#include <utility>
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that evaluates to every value produced by its
   * children.
   * @param <T> The type of reactor producing the reactors to evaluate to.
   */
  template<typename T>
  class Group {
    public:
      using Type = reactor_result_t<reactor_result_t<T>>;

      static constexpr auto is_noexcept =
        is_noexcept_reactor_v<reactor_result_t<T>>;

      /**
       * Constructs a Group.
       * @param producer The reactor producing the reactors to evaluate to.
       */
      template<typename TF, typename = std::enable_if_t<
        !std::is_base_of_v<Group, std::decay_t<TF>>>>
      Group(TF&& producer);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      struct Child {
        try_ptr_t<reactor_result_t<T>> m_reactor;
        State m_state;

        template<typename U>
        Child(U&& reactor);
      };
      try_ptr_t<T> m_producer;
      State m_producer_state;
      std::list<Child> m_children;
      typename std::list<Child>::iterator m_position;
      State m_state;
      int m_previous_sequence;
  };

  template<typename T, typename = std::enable_if_t<
    !std::is_base_of_v<Group<to_reactor_t<T>>, std::decay_t<T>>>>
  Group(T&&) -> Group<to_reactor_t<T>>;

  /**
   * Groups the reactors produced by its child.
   * @param producer The reactor producing the reactors to evaluate to.
   */
  template<typename T>
  auto group(T&& producer) {
    return Group(std::forward<T>(producer));
  }

  template<typename T>
  template<typename U>
  Group<T>::Child::Child(U&& reactor)
    : m_reactor(std::forward<U>(reactor)),
      m_state(State::EMPTY) {}

  template<typename T>
  template<typename TF, typename>
  Group<T>::Group(TF&& producer)
    : m_producer(std::forward<TF>(producer)),
      m_producer_state(State::EMPTY),
      m_position(m_children.end()),
      m_state(State::EMPTY),
      m_previous_sequence(-1) {}

  template<typename T>
  State Group<T>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    if(is_empty(m_state)) {
      m_state = State::EMPTY;
    } else {
      m_state = State::NONE;
    }
    if(!is_complete(m_producer_state)) {
      auto producer_state = m_producer->commit(sequence);
      if(has_evaluation(producer_state) ||
          is_empty(m_producer_state) && !is_empty(producer_state)) {
        try {
          m_children.emplace_back(m_producer->eval());
        } catch(const std::exception&) {}
      }
      m_producer_state = producer_state;
      if(has_continuation(producer_state)) {
        m_state = combine(m_state, State::CONTINUE);
      } else if(is_complete(producer_state) && (m_children.empty() ||
          m_children.size() == 1 && is_complete(m_children.front().m_state))) {
        m_state = combine(m_state, State::COMPLETE);
      }
    }
    auto i = [&] {
      if(m_position == m_children.end() ||
          std::next(m_position) == m_children.end()) {
        return m_children.begin();
      }
      return std::next(m_position);
    }();
    auto end = [&] {
      if(m_children.size() < 2) {
        return m_children.end();
      } else {
        return m_position;
      }
    }();
    while(i != end) {
      auto& child = *i;
      if(is_complete(child.m_state)) {
        i = m_children.erase(i);
      } else {
        auto child_state = child.m_reactor->commit(sequence);
        if(has_evaluation(child_state) ||
            is_empty(child.m_state) && !is_empty(child_state)) {
          m_state = reset(m_state, State::EMPTY);
          m_state = combine(m_state, State::EVALUATED);
          if(m_children.size() > 1) {
            m_state = combine(m_state, State::CONTINUE);
          }
        }
        child.m_state = child_state;
        if(has_continuation(child_state)) {
          m_state = combine(m_state, State::CONTINUE);
        } else if(is_complete(child_state)) {
          if(m_children.size() == 1 && is_complete(m_producer_state)) {
            m_state = combine(m_state, State::COMPLETE);
          }
        }
        if(has_evaluation(m_state)) {
          m_position = i;
          break;
        }
        ++i;
      }
      if(i == m_children.end()) {
        i = m_children.begin();
      }
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename T>
  eval_result_t<typename Group<T>::Type> Group<T>::eval()
      const noexcept(is_noexcept) {
    return m_position->m_reactor->eval();
  }
}

#endif

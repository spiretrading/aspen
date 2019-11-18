#ifndef ASPEN_GROUP_HPP
#define ASPEN_GROUP_HPP
#include <list>
#include <optional>
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
        reactor_result_t<T> m_reactor;
        bool m_is_complete;

        template<typename U>
        Child(U&& reactor);
      };
      std::optional<T> m_producer;
      std::list<Child> m_children;
      typename std::list<Child>::iterator m_position;
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
      m_is_complete(false) {}

  template<typename T>
  template<typename TF, typename>
  Group<T>::Group(TF&& producer)
    : m_producer(std::forward<TF>(producer)),
      m_position(m_children.end()) {}

  template<typename T>
  State Group<T>::commit(int sequence) noexcept {
    auto state = [&] {
      if(m_producer.has_value()) {
        auto producer_state = m_producer->commit(sequence);
        if(has_evaluation(producer_state)) {
          try {
            m_children.emplace_back(m_producer->eval());
          } catch(...) {}
        }
        if(has_continuation(producer_state)) {
          return State::CONTINUE;
        }
        if(is_complete(producer_state)) {
          m_producer = std::nullopt;
          if(m_children.empty() ||
              m_children.size() == 1 && m_children.front().m_is_complete) {
            return State::COMPLETE;
          }
        }
      }
      return State::NONE;
    }();
    auto start = [&] {
      if(m_position == m_children.end() ||
          std::next(m_position) == m_children.end()) {
        return m_children.begin();
      }
      return std::next(m_position);
    }();
    auto looped = false;
    for(auto i = start; !looped || i != start;) {
      auto& child = *i;
      if(child.m_is_complete) {
        i = m_children.erase(i);
      } else {
        auto child_state = child.m_reactor.commit(sequence);
        if(has_continuation(child_state)) {
          state = combine(state, State::CONTINUE);
        } else if(is_complete(child_state)) {
          child.m_is_complete = true;
          if(m_children.size() == 1 && !m_producer.has_value()) {
            state = combine(state, State::COMPLETE);
          }
        }
        if(has_evaluation(child_state)) {
          state = combine(state, State::EVALUATED);
          if(m_children.size() > 1) {
            state = combine(state, State::CONTINUE);
          }
          m_position = i;
          break;
        } else {
          ++i;
        }
      }
      if(i == m_children.end()) {
        looped = true;
        i = m_children.begin();
      }
    }
    return state;
  }

  template<typename T>
  eval_result_t<typename Group<T>::Type> Group<T>::eval()
      const noexcept(is_noexcept) {
    return m_position->m_reactor.eval();
  }
}

#endif

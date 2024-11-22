export module Aspen:Group;

import <cstdint>;
import <utility>;
import :State;
import :Traits;

export namespace Aspen {

  /**
   * Implements a reactor that evaluates two children concurrently.
   * @param <A> The first reactor's type.
   * @param <B> The second reactor's type.
   */
  template<typename A, typename B>
  class Group {
    public:
      using Type = reactor_result_t<A>;
      static constexpr auto is_noexcept = is_noexcept_reactor_v<A> &&
        is_noexcept_reactor_v<B>;

      /**
       * Constructs a Group.
       * @param first The first reactor to commit.
       * @param second The second reactor to commit.
       */
      template<typename AF, typename BF>
      Group(AF&& first, BF&& second);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      A m_first;
      B m_second;
      std::uint8_t m_is_complete: 2;
      std::uint8_t m_current: 1;
      std::uint8_t m_position: 2;

      std::uint8_t next_position() const;
      bool is_complete(std::uint8_t position) const;
  };

  template<typename A, typename B>
  Group(A&&, B&&) -> Group<to_reactor_t<A>, to_reactor_t<B>>;

  /**
   * Groups two reactors together to be evaluated concurrently.
   * @param first The first reactor to commit.
   * @param second The second reactor to commit.
   */
  template<typename A, typename B>
  auto group(A&& first, B&& second) {
    return Group(std::forward<A>(first), std::forward<B>(second));
  }

  /**
   * Groups a series of reactors together.
   * @param first The first reactor to commit.
   * @param second The second reactor to commit.
   */
  template<typename A, typename B, typename... C>
  auto group(A&& first, B&& second, C&&... remainder) {
    return Group(std::forward<A>(first),
      group(std::forward<B>(second), std::forward<C>(remainder)...));
  }

  template<typename A, typename B>
  template<typename AF, typename BF>
  Group<A, B>::Group(AF&& first, BF&& second)
    : m_first(std::forward<AF>(first)),
      m_second(std::forward<BF>(second)),
      m_is_complete(0),
      m_current(0),
      m_position(0) {}

  template<typename A, typename B>
  State Group<A, B>::commit(int sequence) noexcept {
    if(m_position != m_current && is_complete(m_position)) {
      m_position = next_position();
    }
    auto state = State::NONE;
    if(m_is_complete != 3) {
      auto start = m_position;
      while(true) {
        if(is_complete(m_position)) {
          m_position = next_position();
        } else {
          auto child_state = [&] {
            if(m_position == 0) {
              return m_first.commit(sequence);
            }
            return m_second.commit(sequence);
          }();
          if(has_continuation(child_state)) {
            state = combine(state, State::CONTINUE);
          } else if(Aspen::is_complete(child_state)) {
            m_is_complete |= 1 << m_position;
          }
          if(has_evaluation(child_state)) {
            state = combine(state, State::EVALUATED);
            if(!is_complete(next_position())) {
              state = combine(state, State::CONTINUE);
            }
            m_current = m_position;
            m_position = next_position();
            break;
          } else {
            m_position = next_position();
          }
        }
        if(m_position == start) {
          break;
        }
      }
    }
    if(m_is_complete == 3) {
      state = combine(state, State::COMPLETE);
    }
    return state;
  }

  template<typename A, typename B>
  eval_result_t<typename Group<A, B>::Type> Group<A, B>::eval()
      const noexcept(is_noexcept) {
    if(m_current == 0) {
      return m_first.eval();
    } else {
      return m_second.eval();
    }
  }

  template<typename A, typename B>
  std::uint8_t Group<A, B>::next_position() const {
    return (m_position + 1) % 2;
  }

  template<typename A, typename B>
  bool Group<A, B>::is_complete(std::uint8_t position) const {
    return m_is_complete & (1 << position);
  }
}

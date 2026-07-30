#ifndef ASPEN_CHAIN_HPP
#define ASPEN_CHAIN_HPP
#include <concepts>
#include <cstdint>
#include <optional>
#include <utility>
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that produces values from one reactor until it
   * completes and then produces values from another reactor.
   * @param <A> The first reactor's type.
   * @param <B> The second reactor's type.
   */
  template<IsReactor A, IsReactorOf<reactor_result_t<A>> B>
  class Chain {
    public:

      /** The type to evaluate to. */
      using Type = reactor_result_t<A>;

      /** The type returned by an evaluation. */
      using Result = common_evaluation_t<A, B>;

      /** Whether an evaluation is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_evaluation_v<A, B>;

      /**
       * Constructs a Chain.
       * @param initial The reactor to initially evaluate to.
       * @param continuation The reactor to evaluate to thereafter.
       */
      template<typename AF, typename BF> requires
        std::constructible_from<A, AF> && std::constructible_from<B, BF>
      Chain(AF&& initial, BF&& continuation);

      State commit(std::uint64_t sequence) noexcept;
      Result eval() const noexcept(is_noexcept);

    private:
      enum class Status : char {
        START,
        INITIAL,
        TRANSITIONING,
        CONTINUATION
      };
      std::optional<A> m_initial;
      [[no_unique_address]]
      B m_continuation;
      Status m_status;
  };

  template<typename A, typename B>
  Chain(A&&, B&&) -> Chain<to_reactor_t<A>, to_reactor_t<B>>;

  /**
   * Chains two reactors together so that one runs after the other.
   * @param initial The reactor to initially evaluate to.
   * @param continuation The reactor to evaluate to thereafter.
   * @return A reactor evaluating to one and then the other.
   */
  template<typename A, typename B> requires IsReactor<to_reactor_t<A>> &&
    IsReactorOf<to_reactor_t<B>, reactor_result_t<A>>
  auto chain(A&& initial, B&& continuation) {
    return Chain(std::forward<A>(initial), std::forward<B>(continuation));
  }

  /**
   * Chains a series of reactors together.
   * @param initial The reactor to initially evaluate to.
   * @param continuation The reactor to evaluate to thereafter.
   * @param remainder The reactors to evaluate to in turn.
   * @return A reactor evaluating to each of them in turn.
   */
  template<typename A, typename B, typename... C> requires
    IsReactor<to_reactor_t<A>> &&
    IsReactorOf<to_reactor_t<B>, reactor_result_t<A>>
  auto chain(A&& initial, B&& continuation, C&&... remainder) {
    return Chain(std::forward<A>(initial),
      chain(std::forward<B>(continuation), std::forward<C>(remainder)...));
  }

  template<IsReactor A, IsReactorOf<reactor_result_t<A>> B>
  template<typename AF, typename BF> requires
    std::constructible_from<A, AF> && std::constructible_from<B, BF>
  Chain<A, B>::Chain(AF&& initial, BF&& continuation)
    : m_initial(std::forward<AF>(initial)),
      m_continuation(std::forward<BF>(continuation)),
      m_status(Status::START) {}

  template<IsReactor A, IsReactorOf<reactor_result_t<A>> B>
  State Chain<A, B>::commit(std::uint64_t sequence) noexcept {
    if(m_status == Status::START) {
      auto state = m_initial->commit(sequence);
      if(has_evaluation(state)) {
        if(is_complete(state)) {
          m_status = Status::TRANSITIONING;
          return State::CONTINUE_EVALUATED;
        }
        m_status = Status::INITIAL;
        return state;
      } else if(is_complete(state)) {
        m_status = Status::CONTINUATION;
        m_initial = std::nullopt;
        return m_continuation.commit(sequence);
      } else {
        return state;
      }
    } else if(m_status == Status::INITIAL) {
      auto state = m_initial->commit(sequence);
      if(is_complete(state)) {
        m_status = Status::TRANSITIONING;
        if(has_evaluation(state)) {
          return State::CONTINUE_EVALUATED;
        }
      } else {
        return state;
      }
    } else if(m_status == Status::CONTINUATION) {
      return m_continuation.commit(sequence);
    }
    auto state = m_continuation.commit(sequence);
    if(has_evaluation(state)) {
      m_status = Status::CONTINUATION;
      m_initial = std::nullopt;
    }
    return state;
  }

  template<IsReactor A, IsReactorOf<reactor_result_t<A>> B>
  typename Chain<A, B>::Result Chain<A, B>::eval() const noexcept(is_noexcept) {
    if(m_status == Status::CONTINUATION) {
      return m_continuation.eval();
    }
    return m_initial->eval();
  }
}

#endif

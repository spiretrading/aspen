#ifndef ASPEN_WHEN_HPP
#define ASPEN_WHEN_HPP
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Implements a reactor that begins evaluating its child when a condition is
   * true.
   * @param <C> The type of reactor used as the condition.
   * @param <T> The type of reactor to evaluate to.
   */
  template<typename C, typename T>
  class When {
    public:
      using Type = reactor_result_t<T>;
      static constexpr auto is_noexcept = is_noexcept_reactor_v<C> &&
        is_noexcept_reactor_v<T>;

      /**
       * Constructs a When.
       * @param condition The condition to evaluate.
       * @param series The series to evaluate when the condition evaluates to
       *        <code>true</i>.
       */
      template<typename CF, typename TF>
      When(CF&& condition, TF&& series);

      State commit(int sequence) noexcept;

      const Type& eval() const noexcept(is_noexcept);
  };

  template<typename C, typename T>
  When(C&&, T&&) -> When<to_reactor_t<C>, to_reactor_t<T>>;

  /**
   * Returns a reactor that begins evaluating its child when a condition is
   * true.
   * @param condition The condition to evaluate.
   * @param The series to evaluate when the condition evaluates to
   *        <code>true</i>.
   */
  template<typename C, typename T>
  auto when(C&& condition, T&& series) {
    return When(std::forward<C>(condition), std::forward<T>(series));
  }

  template<typename C, typename T>
  template<typename CF, typename TF>
  When<C, T>::When(CF&& condition, TF&& series) {}

  template<typename C, typename T>
  State When<C, T>::commit(int sequence) noexcept {
    return {};
  }

  template<typename C, typename T>
  const typename When<C, T>::Type& When<C, T>::eval() const noexcept(
      is_noexcept) {
    throw std::runtime_error("Not implemented.");
  }
}

#endif

#ifndef ASPEN_CONVERSIONS_HPP
#define ASPEN_CONVERSIONS_HPP
#include <concepts>
#include <cstdint>
#include <type_traits>
#include <utility>
#include "Aspen/Maybe.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Evaluates its child reactor and then applies a conversion function to the
   * result.
   * @param <R> The type of reactor to perform the conversion to.
   * @param <F> The conversion function to apply to the reactor.
   */
  template<IsReactor R, std::invocable<decltype(std::declval<R>().eval())> F>
  class ConversionReactor {
    public:

      /** The type to evaluate to. */
      using Type =
        std::decay_t<decltype(std::declval<F>()(std::declval<R>().eval()))>;

      /** Whether an evaluation is noexcept. */
      static constexpr auto is_noexcept =
        noexcept(std::declval<F>()(std::declval<R>().eval()));

      /**
       * Constructs a ConversionReactor.
       * @param reactor The reactor to apply the conversion to.
       * @param conversion The conversion to perform.
       */
      template<typename RF, typename FF> requires
        std::constructible_from<R, RF> && std::constructible_from<F, FF>
      ConversionReactor(RF&& reactor, FF&& conversion);

      State commit(std::uint64_t sequence) noexcept;
      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      [[no_unique_address]]
      R m_reactor;
      [[no_unique_address]]
      F m_conversion;
      try_maybe_t<Type, !is_noexcept> m_value;
  };

  template<typename R, typename F>
  ConversionReactor(R&&, F&&) ->
    ConversionReactor<std::decay_t<R>, std::decay_t<F>>;

  /**
   * Performs a static_cast on the result of a reactor.
   * @param reactor The reactor whose evaluation is to be cast.
   * @return A reactor evaluating to the <i>reactor</i> cast to the given type.
   */
  template<typename T>
  auto static_reactor_cast(auto&& reactor) {
    if constexpr(std::is_same_v<T, reactor_result_t<decltype(reactor)>>) {
      return std::forward<decltype(reactor)>(reactor);
    } else {
      return ConversionReactor(std::forward<decltype(reactor)>(reactor),
        [] (auto&& value) noexcept {
          return static_cast<T>(std::forward<decltype(value)>(value));
        });
    }
  }

  template<IsReactor R, std::invocable<decltype(std::declval<R>().eval())> F>
  template<typename RF, typename FF> requires
    std::constructible_from<R, RF> && std::constructible_from<F, FF>
  ConversionReactor<R, F>::ConversionReactor(RF&& reactor, FF&& conversion)
    : m_reactor(std::forward<RF>(reactor)),
      m_conversion(std::forward<FF>(conversion)) {}

  template<IsReactor R, std::invocable<decltype(std::declval<R>().eval())> F>
  State ConversionReactor<R, F>::commit(std::uint64_t sequence) noexcept {
    auto state = m_reactor.commit(sequence);
    if(has_evaluation(state)) {
      m_value = try_call([&] () noexcept(is_noexcept) {
        return m_conversion(m_reactor.eval());
      });
    }
    return state;
  }

  template<IsReactor R, std::invocable<decltype(std::declval<R>().eval())> F>
  eval_result_t<typename ConversionReactor<R, F>::Type>
      ConversionReactor<R, F>::eval() const noexcept(is_noexcept) {
    return *m_value;
  }
}

#endif

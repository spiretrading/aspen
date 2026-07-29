#ifndef ASPEN_CONVERSIONS_HPP
#define ASPEN_CONVERSIONS_HPP
#include <cstdint>
#include <type_traits>
#include <utility>
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Evaluates its child reactor and then applies a conversion function to
   * to the result.
   * @param <R> The type of reactor to perform the conversion to.
   * @param <F> The conversion function to apply to the reactor.
   */
  template<typename R, typename F>
  class ConversionReactor {
    public:
      using Type = decltype(std::declval<F>()(std::declval<R>().eval()));
      static constexpr auto is_noexcept =
        noexcept(std::declval<F>()(std::declval<R>().eval()));

      /**
       * Constructs a ConversionReactor.
       * @param reactor The reactor to apply the conversion to.
       * @param conversion The conversion to perform.
       */
      template<typename RF, typename FF>
      ConversionReactor(RF&& reactor, FF&& conversion);

      State commit(std::uint64_t sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      R m_reactor;
      F m_conversion;
      try_maybe_t<Type, !is_noexcept> m_value;
  };

  template<typename R, typename F>
  ConversionReactor(R&&, F&&) ->
    ConversionReactor<std::decay_t<R>, std::decay_t<F>>;

  /** Performs a static_cast on the result of a reactor. */
  template<typename T, typename R>
  decltype(auto) static_reactor_cast(R&& reactor) {
    if constexpr(std::is_same_v<T, reactor_result_t<R>>) {
      return std::forward<R>(reactor);
    } else {
      return ConversionReactor(std::forward<R>(reactor),
        [] (auto&& value) noexcept {
          return static_cast<T>(std::forward<decltype(value)>(value));
        });
    }
  }

  template<typename R, typename F>
  template<typename RF, typename FF>
  ConversionReactor<R, F>::ConversionReactor(RF&& reactor, FF&& conversion)
    : m_reactor(std::forward<RF>(reactor)),
      m_conversion(std::forward<FF>(conversion)) {}

  template<typename R, typename F>
  State ConversionReactor<R, F>::commit(std::uint64_t sequence) noexcept {
    auto state = m_reactor.commit(sequence);
    if(has_evaluation(state)) {
      if constexpr(is_noexcept && !std::is_same_v<Type, void>) {
        *m_value = m_conversion(m_reactor.eval());
      } else {
        m_value = try_call([&] () noexcept(is_noexcept) {
          return m_conversion(m_reactor.eval());
        });
      }
    }
    return state;
  }

  template<typename R, typename F>
  eval_result_t<typename ConversionReactor<R, F>::Type>
      ConversionReactor<R, F>::eval() const noexcept(is_noexcept) {
    return *m_value;
  }
}

#endif

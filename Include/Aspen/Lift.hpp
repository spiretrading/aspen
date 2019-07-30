#ifndef ASPEN_LIFT_HPP
#define ASPEN_LIFT_HPP
#include <tuple>
#include <type_traits>
#include <utility>
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"
#include "Aspen/StaticCommitHandler.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {
namespace Details {
  template<typename T>
  decltype(auto) deref(T& value) noexcept {
    if constexpr(is_reactor_pointer_v<T>) {
      return *value;
    } else {
      return value;
    }
  }
}

  /**
   * A reactor that applies a function to its parameters.
   * @param <F> The type of function to apply.
   * @param <P> The type of arguments to apply the function to.
   */
  template<typename F, typename... A>
  class Lift {
    public:
      using Type = std::decay_t<
        std::invoke_result_t<F, const try_maybe_t<reactor_result_t<A>,
        !is_noexcept_reactor_v<A>>& ...>>;

      /** The type of function to apply. */
      using Function = F;

      /** A tuple containing the list of arguments to apply to the function. */
      using Arguments = std::tuple<A...>;

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_function_v<F,
        const try_maybe_t<reactor_result_t<A>,
        !is_noexcept_reactor_v<A>>& ...> &&
        std::conjunction_v<is_noexcept_reactor<A>...>;

      /**
       * Constructs a function reactor.
       * @param function The function to apply.
       * @param arguments The arguments to apply the <i>function</i> to.
       */
      template<typename FF, typename AF, typename... AR>
      Lift(FF&& function, AF&& argument, AR&&... arguments);

      Lift(const Lift& lift);

      Lift(Lift&& lift);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

      Lift& operator =(const Lift& lift);

      Lift& operator =(Lift&& lift);

    private:
      Function m_function;
      Arguments m_arguments;
      StaticCommitHandler<decltype(&Details::deref(std::declval<A&>()))...>
        m_handler;
      try_maybe_t<Type, std::is_same_v<Type, void> || !is_noexcept> m_value;
      State m_state;
      int m_previous_sequence;
  };

  /**
   * Specialization for functions that have no parameters.
   * @param <F> The type of function to apply.
   */
  template<typename F>
  class Lift<F> {
    public:
      using Type = std::decay_t<std::invoke_result_t<F>>;
      using Function = F;
      using Arguments = std::tuple<>;
      static constexpr auto is_noexcept = is_noexcept_function_v<F>;

      /**
       * Constructs a function reactor.
       * @param function The function to apply.
       */
      template<typename FF>
      explicit Lift(FF&& function);

      Lift(const Lift& lift);

      Lift(Lift&& lift);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

      Lift& operator =(const Lift& lift);

      Lift& operator =(Lift&& lift);

    private:
      Function m_function;
      try_maybe_t<Type, std::is_same_v<Type, void> || !is_noexcept> m_value;
      State m_state;
  };

  template<typename F, typename AF, typename... AR>
  Lift(F&&, AF&&, AR&&...) -> Lift<std::decay_t<F>, std::decay_t<AF>,
    std::decay_t<AR>...>;

  template<typename F>
  Lift(F&&) -> Lift<std::decay_t<F>>;

  /**
   * Lifts a function to operate on reactors.
   * @param function The function to lift.
   * @param arguments The reactors used as arguments to the function.
   */
  template<typename F, typename... A>
  auto lift(F&& function, A&&... arguments) {
    return Lift(std::forward<F>(function), std::forward<A>(arguments)...);
  }

  template<typename F, typename... A>
  template<typename FF, typename AF, typename... AR>
  Lift<F, A...>::Lift(FF&& function, AF&& argument, AR&&... arguments)
    : m_function(std::forward<FF>(function)),
      m_arguments(std::forward<AF>(argument), std::forward<AR>(arguments)...),
      m_handler(std::apply([] (auto&&... arguments) {
        return std::make_tuple(&Details::deref(arguments)...);
      }, m_arguments)),
      m_state(State::EMPTY),
      m_previous_sequence(-1) {}

  template<typename F, typename... A>
  Lift<F, A...>::Lift(const Lift& lift)
      : m_function(lift.m_function),
        m_arguments(lift.m_arguments),
        m_handler(std::apply([] (auto&&... arguments) {
          return std::make_tuple(&Details::deref(arguments)...);
        }, m_arguments)),
        m_value(lift.m_value),
        m_state(lift.m_state),
        m_previous_sequence(lift.m_previous_sequence) {
    m_handler.transfer(lift.m_handler);
  }

  template<typename F, typename... A>
  Lift<F, A...>::Lift(Lift&& lift)
      : m_function(std::move(lift.m_function)),
        m_arguments(std::move(lift.m_arguments)),
        m_handler(std::apply([] (auto&&... arguments) {
          return std::make_tuple(&Details::deref(arguments)...);
        }, m_arguments)),
        m_value(std::move(lift.m_value)),
        m_state(std::move(lift.m_state)),
        m_previous_sequence(std::move(lift.m_previous_sequence)) {
    m_handler.transfer(lift.m_handler);
  }

  template<typename F, typename... A>
  State Lift<F, A...>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    auto state = m_handler.commit(sequence);
    if(has_evaluation(state) ||
        is_complete(state) && !is_empty(state) && is_empty(m_state)) {
      try {
        if constexpr(!std::is_same_v<Type, void>) {
          m_value = std::apply(
            [&] (const auto&... arguments) {
              return m_function(Details::deref(arguments).eval()...);
            }, m_arguments);
        } else {
          std::apply(
            [&] (const auto&... arguments) {
              return m_function(Details::deref(arguments).eval()...);
            }, m_arguments);
        }
      } catch(const std::exception&) {
        if constexpr(!is_noexcept) {
          m_value = std::current_exception();
        }
      }
      state = combine(state, State::EVALUATED);
    }
    m_previous_sequence = sequence;
    m_state = state;
    return m_state;
  }

  template<typename F, typename... A>
  eval_result_t<typename Lift<F, A...>::Type> Lift<F, A...>::eval() const
      noexcept(is_noexcept){
    if constexpr(std::is_same_v<Type, void> && !is_noexcept) {
      m_value.get();
    } else {
      return m_value;
    }
  }

  template<typename F, typename... A>
  Lift<F, A...>& Lift<F, A...>::operator =(const Lift& lift) {
    m_function = lift.m_function;
    m_arguments = lift.m_arguments;
    m_handler = StaticCommitHandler(std::apply(
      [] (auto&&... arguments) {
        return std::make_tuple(&Details::deref(arguments)...);
      }, m_arguments));
    m_handler.transfer(lift.m_handler);
    m_value = lift.m_value;
    m_state = lift.m_state;
    m_previous_sequence = lift.m_previous_sequence;
    return *this;
  }

  template<typename F, typename... A>
  Lift<F, A...>& Lift<F, A...>::operator =(Lift&& lift) {
    m_function = std::move(lift.m_function);
    m_arguments = std::move(lift.m_arguments);
    m_handler = StaticCommitHandler(std::apply(
      [] (auto&&... arguments) {
        return std::make_tuple(&Details::deref(arguments)...);
      }, m_arguments));
    m_handler.transfer(lift.m_handler);
    m_value = std::move(lift.m_value);
    m_state = std::move(lift.m_state);
    m_previous_sequence = std::move(lift.m_previous_sequence);
    return *this;
  }

  template<typename F>
  template<typename FF>
  Lift<F>::Lift(FF&& function)
    : m_function(std::forward<FF>(function)),
      m_state(State::EMPTY) {}

  template<typename F>
  State Lift<F>::commit(int sequence) noexcept {
    if(is_complete(m_state)) {
      return m_state;
    }
    try {
      if constexpr(!std::is_same_v<Type, void>) {
        m_value = m_function();
      } else {
        m_function();
      }
    } catch(const std::exception&) {
      if constexpr(!is_noexcept) {
        m_value = std::current_exception();
      }
    }
    m_state = State::COMPLETE_EVALUATED;
    return m_state;
  }

  template<typename F>
  eval_result_t<typename Lift<F>::Type> Lift<F>::eval()
      const noexcept(is_noexcept) {
    return *m_value;
  }
}

#endif

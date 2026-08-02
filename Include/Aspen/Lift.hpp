#ifndef ASPEN_LIFT_HPP
#define ASPEN_LIFT_HPP
#include <cassert>
#include <concepts>
#include <cstdint>
#include <exception>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include "Aspen/Maybe.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/StaticCommitHandler.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Stores the result of a function evaluation within a reactor.
   * @param <T> The type of result produced by the function.
   */
  template<typename T>
  struct FunctionEvaluation {

    /** The result of the function. */
    using Type = T;

    /** The value returned by the function. */
    std::optional<Type> m_value;

    /** The state of the reactor after the function evaluation. */
    State m_state;

    /** Constructs an uninitialized evaluation. */
    FunctionEvaluation();

    /**
     * Constructs an evaluation resulting in a value and an EVALUATED state.
     * @param value The value returned by the function.
     */
    FunctionEvaluation(Type value);

    /**
     * Constructs an evaluation resulting in a value if one is provided.
     * @param value The value returned by the function.
     */
    FunctionEvaluation(std::optional<Type> value);

    /**
     * Constructs an evaluation resulting in a value and an update.
     * @param value The value returned by the function.
     * @param state The State of the reactor after the evaluation.
     */
    FunctionEvaluation(Type value, State state);

    /**
     * Constructs an evaluation resulting in a value and an update.
     * @param value The value returned by the function.
     * @param state The State of the reactor after the evaluation.
     */
    FunctionEvaluation(std::optional<Type> value, State state);

    /**
     * Constructs an evaluation resulting in just an update.
     * @param state The State of the reactor after the evaluation.
     */
    FunctionEvaluation(State state);
  };

  /**
   * Stores the result of a function evaluation that is able to result in an
   * exception.
   * @param <T> The result of the function.
   */
  template<typename T>
  struct FunctionEvaluation<Maybe<T>> {

    /** The result of the function. */
    using Type = T;

    /** The value returned by the function. */
    std::optional<Maybe<Type>> m_value;

    /** The state of the reactor after the function evaluation. */
    State m_state;

    /** Constructs an uninitialized evaluation. */
    FunctionEvaluation();

    /**
     * Constructs an evaluation resulting in a value and an EVALUATED state.
     * @param value The value returned by the function.
     */
    FunctionEvaluation(Maybe<Type> value);

    /**
     * Constructs an evaluation resulting in a value and an EVALUATED state.
     * @param value The value returned by the function.
     */
    FunctionEvaluation(Type value);

    /**
     * Constructs an evaluation resulting in a value if one is provided.
     * @param value The value returned by the function.
     */
    FunctionEvaluation(std::optional<Maybe<Type>> value);

    /**
     * Constructs an evaluation resulting in a value if one is provided.
     * @param value The value returned by the function.
     */
    FunctionEvaluation(std::optional<Type> value);

    /**
     * Constructs an evaluation resulting in a value and an update.
     * @param value The value returned by the function.
     * @param state The State of the reactor after the evaluation.
     */
    FunctionEvaluation(Maybe<Type> value, State state);

    /**
     * Constructs an evaluation resulting in a value and an update.
     * @param value The value returned by the function.
     * @param state The State of the reactor after the evaluation.
     */
    FunctionEvaluation(Type value, State state);

    /**
     * Constructs an evaluation resulting in a value and an update.
     * @param value The value returned by the function.
     * @param state The State of the reactor after the evaluation.
     */
    FunctionEvaluation(std::optional<Maybe<Type>> value, State state);

    /**
     * Constructs an evaluation resulting in a value and an update.
     * @param value The value returned by the function.
     * @param state The State of the reactor after the evaluation.
     */
    FunctionEvaluation(std::optional<Type> value, State state);

    /**
     * Constructs an evaluation resulting in just an update.
     * @param state The State of the reactor after the evaluation.
     */
    FunctionEvaluation(State state);
  };

  template<>
  struct FunctionEvaluation<void> {

    /** The result of the function. */
    using Type = void;

    /** Whether the function produced an evaluation. */
    bool m_value;

    /** The state of the reactor after the function evaluation. */
    State m_state;

    /** Constructs an uninitialized evaluation. */
    FunctionEvaluation();

    /**
     * Constructs an evaluation resulting in an update.
     * @param state The State of the reactor after the evaluation.
     */
    FunctionEvaluation(State state);
  };

  template<>
  struct FunctionEvaluation<Maybe<void>> {

    /** The result of the function. */
    using Type = void;

    std::optional<Maybe<Type>> m_value;
    State m_state;

    FunctionEvaluation();
    FunctionEvaluation(Maybe<Type> value);
    FunctionEvaluation(std::optional<Maybe<Type>> value);
    FunctionEvaluation(Maybe<Type> value, State state);
    FunctionEvaluation(std::optional<Maybe<Type>> value, State state);
    FunctionEvaluation(State state);
  };

namespace Details {
  template<typename T>
  struct function_reactor_result {
    using type = std::decay_t<T>;
  };

  template<typename T>
  struct function_reactor_result<std::optional<T>> {
    using type = T;
  };

  template<typename T>
  struct function_reactor_result<std::optional<Maybe<T>>> {
    using type = T;
  };

  template<typename T>
  struct function_reactor_result<FunctionEvaluation<T>> {
    using type = typename FunctionEvaluation<T>::Type;
  };

  template<typename T>
  struct function_reactor_result<Maybe<T>> {
    using type = T;
  };

  template<typename T>
  using function_reactor_result_t =
    typename function_reactor_result<std::remove_cvref_t<T>>::type;

  template<typename T>
  struct is_maybe_result : std::false_type {};

  template<typename T>
  struct is_maybe_result<Maybe<T>> : std::true_type {};

  template<typename T>
  struct is_maybe_result<std::optional<Maybe<T>>> : std::true_type {};

  template<typename T>
  struct is_maybe_result<FunctionEvaluation<Maybe<T>>> : std::true_type {};

  template<typename T>
  constexpr auto is_maybe_result_v =
    is_maybe_result<std::remove_cvref_t<T>>::value;

  template<typename T>
  struct is_function_evaluation : std::false_type {};

  template<typename T>
  struct is_function_evaluation<FunctionEvaluation<T>> : std::true_type {};

  template<typename T>
  constexpr auto is_function_evaluation_v =
    is_function_evaluation<std::remove_cvref_t<T>>::value;

  template<typename T>
  auto make_evaluation(auto&& result) {
    using Result = std::remove_cvref_t<decltype(result)>;
    if constexpr(is_function_evaluation_v<Result>) {
      return std::forward<decltype(result)>(result);
    } else if constexpr(is_maybe_result_v<Result>) {
      return FunctionEvaluation<Maybe<T>>(
        std::forward<decltype(result)>(result));
    } else {
      return FunctionEvaluation<T>(std::forward<decltype(result)>(result));
    }
  }

  decltype(auto) eval_argument(const auto& reactor) {
    return try_call([&] () noexcept(noexcept(reactor.eval())) ->
        decltype(auto) {
      return reactor.eval();
    });
  }

  template<typename T>
  struct FunctionEvaluator {
    State operator ()(auto& value, auto& function, const auto& pack) const {
      return apply([&] (const auto&... arguments) {
        if constexpr(std::is_same_v<
            std::decay_t<decltype(function(eval_argument(arguments)...))>, T>) {
          value = function(eval_argument(arguments)...);
          return State::EVALUATED;
        } else {
          auto evaluation =
            make_evaluation<T>(function(eval_argument(arguments)...));
          if(evaluation.m_value) {
            value = std::move(*evaluation.m_value);
          }
          return evaluation.m_state;
        }
      }, pack);
    }
  };

  template<>
  struct FunctionEvaluator<void> {
    State operator ()(auto& value, auto& function, const auto& pack) const {
      return apply([&] (const auto&... arguments) {
        if constexpr(std::is_same_v<std::decay_t<
            decltype(function(eval_argument(arguments)...))>, void>) {
          value = try_call([&] {
            return function(eval_argument(arguments)...);
          });
          return State::EVALUATED;
        } else {
          auto evaluation =
            make_evaluation<void>(function(eval_argument(arguments)...));
          if(evaluation.m_value) {
            if constexpr(std::is_same_v<decltype(evaluation.m_value), bool>) {
              value = Maybe<void>();
            } else {
              value = std::move(*evaluation.m_value);
            }
          }
          return evaluation.m_state;
        }
      }, pack);
    }
  };

  template<typename A>
  using lift_argument_t = decltype(eval_argument(std::declval<const A&>()));

  template<typename F, typename... A>
  struct is_lift_noexcept : std::bool_constant<
    std::is_nothrow_invocable_v<F, lift_argument_t<A>...> &&
    is_noexcept_reactor_v<A...> &&
    !is_maybe_result_v<std::invoke_result_t<F, lift_argument_t<A>...>>> {};

  template<typename F, typename... A>
  constexpr auto is_lift_noexcept_v = is_lift_noexcept<F, A...>::value;
}

  /**
   * A reactor that applies a function to its parameters.
   * @param <F> The type of function to apply.
   * @param <A> The types of reactors whose evaluations the function is applied
   *        to.
   */
  template<typename F, IsReactor... A> requires
    std::invocable<F, Details::lift_argument_t<A>...>
  class Lift {
    public:

      /** The type to evaluate to. */
      using Type = Details::function_reactor_result_t<
        std::invoke_result_t<F, Details::lift_argument_t<A>...>>;

      /** The type of function to apply. */
      using Function = F;

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = Details::is_lift_noexcept_v<F, A...>;

      /**
       * Constructs a function reactor.
       * @param function The function to apply.
       * @param arguments The arguments to apply the <i>function</i> to.
       */
      template<typename FF, typename AF, typename... AR> requires
        std::constructible_from<F, FF>
      Lift(FF&& function, AF&& argument, AR&&... arguments);

      State commit(std::uint64_t sequence) noexcept;
      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      [[no_unique_address]]
      Function m_function;
      StaticCommitHandler<A...> m_handler;
      try_maybe_t<Type, std::is_same_v<Type, void> || !is_noexcept> m_value;
      bool m_has_continuation;

      State invoke();
  };

  /**
   * Specialization for functions that have no parameters.
   * @param <F> The type of function to apply.
   */
  template<std::invocable F>
  class Lift<F> {
    public:

      /** The type to evaluate to. */
      using Type = Details::function_reactor_result_t<std::invoke_result_t<F>>;

      /** The type of function to apply. */
      using Function = F;

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = Details::is_lift_noexcept_v<F>;

      /**
       * Constructs a function reactor.
       * @param function The function to apply.
       */
      template<typename FF> requires std::constructible_from<F, FF>
      explicit Lift(FF&& function);

      State commit(std::uint64_t sequence) noexcept;
      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      [[no_unique_address]]
      Function m_function;
      try_maybe_t<Type, std::is_same_v<Type, void> || !is_noexcept> m_value;

      State invoke();
  };

  template<typename F, typename AF, typename... AR>
  Lift(F&&, AF&&, AR&&...) ->
    Lift<std::decay_t<F>, to_reactor_t<AF>, to_reactor_t<AR>...>;

  template<typename F> requires std::invocable<std::decay_t<F>>
  Lift(F&&) -> Lift<std::decay_t<F>>;

  /**
   * Lifts a function to operate on reactors.
   * @param function The function to lift.
   * @param arguments The reactors used as arguments to the function.
   * @return A reactor applying the <i>function</i> to the <i>arguments</i>.
   */
  template<typename F, typename... A> requires std::invocable<
    std::decay_t<F>, Details::lift_argument_t<to_reactor_t<A>>...>
  auto lift(F&& function, A&&... arguments) {
    return Lift(std::forward<F>(function), std::forward<A>(arguments)...);
  }

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation()
    : m_state(State::NONE) {}

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(Type value)
    : m_value(std::move(value)),
      m_state(State::EVALUATED) {}

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(std::optional<Type> value)
      : m_value(std::move(value)) {
    if(m_value) {
      m_state = State::EVALUATED;
    } else {
      m_state = State::NONE;
    }
  }

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(Type value, State state)
    : FunctionEvaluation(std::optional<Type>(std::move(value)), state) {}

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(
      std::optional<Type> value, State state)
      : m_value(std::move(value)) {
    if(m_value) {
      if(is_complete(state)) {
        m_state = State::COMPLETE_EVALUATED;
      } else if(has_continuation(state)) {
        m_state = State::CONTINUE_EVALUATED;
      } else {
        m_state = State::EVALUATED;
      }
    } else if(is_complete(state)) {
      m_state = State::COMPLETE;
    } else if(has_continuation(state)) {
      m_state = State::CONTINUE;
    } else {
      m_state = State::NONE;
    }
  }

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(State state)
      : m_state(state) {
    assert(!has_evaluation(m_state));
  }

  template<typename T>
  FunctionEvaluation<Maybe<T>>::FunctionEvaluation()
    : m_state(State::NONE) {}

  template<typename T>
  FunctionEvaluation<Maybe<T>>::FunctionEvaluation(Maybe<Type> value)
    : m_value(std::move(value)),
      m_state(State::EVALUATED) {}

  template<typename T>
  FunctionEvaluation<Maybe<T>>::FunctionEvaluation(Type value)
    : FunctionEvaluation(Maybe(std::move(value))) {}

  template<typename T>
  FunctionEvaluation<Maybe<T>>::FunctionEvaluation(
      std::optional<Maybe<Type>> value)
      : m_value(std::move(value)) {
    if(m_value) {
      m_state = State::EVALUATED;
    } else {
      m_state = State::NONE;
    }
  }

  template<typename T>
  FunctionEvaluation<Maybe<T>>::FunctionEvaluation(std::optional<Type> value)
    : FunctionEvaluation(std::optional<Maybe<Type>>(std::move(value))) {}

  template<typename T>
  FunctionEvaluation<Maybe<T>>::FunctionEvaluation(
      Maybe<Type> value, State state)
      : m_value(std::move(value)) {
    if(is_complete(state)) {
      m_state = State::COMPLETE_EVALUATED;
    } else if(has_continuation(state)) {
      m_state = State::CONTINUE_EVALUATED;
    } else {
      m_state = State::EVALUATED;
    }
  }

  template<typename T>
  FunctionEvaluation<Maybe<T>>::FunctionEvaluation(Type value, State state)
    : FunctionEvaluation(Maybe(std::move(value)), state) {}

  template<typename T>
  FunctionEvaluation<Maybe<T>>::FunctionEvaluation(
      std::optional<Maybe<Type>> value, State state)
      : m_value(std::move(value)) {
    if(m_value) {
      if(is_complete(state)) {
        m_state = State::COMPLETE_EVALUATED;
      } else if(has_continuation(state)) {
        m_state = State::CONTINUE_EVALUATED;
      } else {
        m_state = State::EVALUATED;
      }
    } else if(is_complete(state)) {
      m_state = State::COMPLETE;
    } else if(has_continuation(state)) {
      m_state = State::CONTINUE;
    } else {
      m_state = State::NONE;
    }
  }

  template<typename T>
  FunctionEvaluation<Maybe<T>>::FunctionEvaluation(
    std::optional<Type> value, State state)
    : FunctionEvaluation(
        std::optional<Maybe<Type>>(std::move(value)), state) {}

  template<typename T>
  FunctionEvaluation<Maybe<T>>::FunctionEvaluation(State state)
      : m_state(state) {
    assert(!has_evaluation(m_state));
  }

  inline FunctionEvaluation<void>::FunctionEvaluation()
    : m_value(false),
      m_state(State::NONE) {}

  inline FunctionEvaluation<void>::FunctionEvaluation(State state)
    : m_value(has_evaluation(state)),
      m_state(state) {}

  inline FunctionEvaluation<Maybe<void>>::FunctionEvaluation()
    : m_state(State::NONE) {}

  inline FunctionEvaluation<Maybe<void>>::FunctionEvaluation(Maybe<Type> value)
    : m_value(std::move(value)),
      m_state(State::EVALUATED) {}

  inline FunctionEvaluation<Maybe<void>>::FunctionEvaluation(
      std::optional<Maybe<Type>> value)
      : m_value(std::move(value)) {
    if(m_value) {
      m_state = State::EVALUATED;
    } else {
      m_state = State::NONE;
    }
  }

  inline FunctionEvaluation<Maybe<void>>::FunctionEvaluation(
      Maybe<Type> value, State state)
      : m_value(std::move(value)) {
    if(is_complete(state)) {
      m_state = State::COMPLETE_EVALUATED;
    } else if(has_continuation(state)) {
      m_state = State::CONTINUE_EVALUATED;
    } else {
      m_state = State::EVALUATED;
    }
  }

  inline FunctionEvaluation<Maybe<void>>::FunctionEvaluation(
      std::optional<Maybe<Type>> value, State state)
      : m_value(std::move(value)) {
    if(m_value) {
      if(is_complete(state)) {
        m_state = State::COMPLETE_EVALUATED;
      } else if(has_continuation(state)) {
        m_state = State::CONTINUE_EVALUATED;
      } else {
        m_state = State::EVALUATED;
      }
    } else if(is_complete(state)) {
      m_state = State::COMPLETE;
    } else if(has_continuation(state)) {
      m_state = State::CONTINUE;
    } else {
      m_state = State::NONE;
    }
  }

  inline FunctionEvaluation<Maybe<void>>::FunctionEvaluation(State state)
      : m_state(state) {
    assert(!has_evaluation(m_state));
  }

  template<typename F, IsReactor... A> requires
    std::invocable<F, Details::lift_argument_t<A>...>
  template<typename FF, typename AF, typename... AR> requires
    std::constructible_from<F, FF>
  Lift<F, A...>::Lift(FF&& function, AF&& argument, AR&&... arguments)
    : m_function(std::forward<FF>(function)),
      m_handler(std::forward<AF>(argument), std::forward<AR>(arguments)...),
      m_has_continuation(false) {}

  template<typename F, IsReactor... A> requires
    std::invocable<F, Details::lift_argument_t<A>...>
  State Lift<F, A...>::commit(std::uint64_t sequence) noexcept {
    auto state = State::NONE;
    auto children_state = m_handler.commit(sequence);
    if(has_evaluation(children_state) || m_has_continuation) {
      m_has_continuation = false;
      auto invocation = invoke();
      if(invocation == State::NONE) {
        if(is_complete(children_state)) {
          state = State::COMPLETE;
        } else if(has_continuation(children_state)) {
          state = State::CONTINUE;
        }
      } else if(is_complete(invocation)) {
        if(has_evaluation(invocation)) {
          state = State::COMPLETE_EVALUATED;
        } else {
          state = State::COMPLETE;
        }
      } else {
        state = invocation;
        m_has_continuation = has_continuation(invocation);
        if(has_continuation(children_state)) {
          state = combine(state, State::CONTINUE);
        } else if(is_complete(children_state) && !m_has_continuation) {
          state = combine(state, State::COMPLETE);
        }
      }
    } else {
      state = children_state;
    }
    return state;
  }

  template<typename F, IsReactor... A> requires
    std::invocable<F, Details::lift_argument_t<A>...>
  eval_result_t<typename Lift<F, A...>::Type> Lift<F, A...>::eval() const
      noexcept(is_noexcept) {
    return *m_value;
  }

  template<typename F, IsReactor... A> requires
    std::invocable<F, Details::lift_argument_t<A>...>
  State Lift<F, A...>::invoke() {
    if constexpr(is_noexcept) {
      return Details::FunctionEvaluator<Type>()(
        m_value, m_function, m_handler);
    } else {
      try {
        return Details::FunctionEvaluator<Type>()(
          m_value, m_function, m_handler);
      } catch(...) {
        m_value = std::current_exception();
        return State::EVALUATED;
      }
    }
  }

  template<std::invocable F>
  template<typename FF> requires std::constructible_from<F, FF>
  Lift<F>::Lift(FF&& function)
    : m_function(std::forward<FF>(function)) {}

  template<std::invocable F>
  State Lift<F>::commit(std::uint64_t sequence) noexcept {
    auto invocation = invoke();
    if(has_evaluation(invocation)) {
      if(has_continuation(invocation)) {
        return State::CONTINUE_EVALUATED;
      } else {
        return State::COMPLETE_EVALUATED;
      }
    } else if(has_continuation(invocation)) {
      return State::CONTINUE;
    }
    return State::COMPLETE;
  }

  template<std::invocable F>
  eval_result_t<typename Lift<F>::Type> Lift<F>::eval() const
      noexcept(is_noexcept) {
    return *m_value;
  }

  template<std::invocable F>
  State Lift<F>::invoke() {
    if constexpr(is_noexcept) {
      return Details::FunctionEvaluator<Type>()(
        m_value, m_function, std::tuple<>());
    } else {
      try {
        return Details::FunctionEvaluator<Type>()(
          m_value, m_function, std::tuple<>());
      } catch(...) {
        m_value = std::current_exception();
        return State::EVALUATED;
      }
    }
  }
}

#endif

#ifndef ASPEN_LIFT_HPP
#define ASPEN_LIFT_HPP
#include <cassert>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"
#include "Aspen/StaticCommitHandler.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Stores the result of a function evaluation within a reactor.
   * @param <T> The result of the function.
   */
  template<typename T>
  struct FunctionEvaluation {
    using Type = T;

    /** The value returned by the function. */
    std::optional<Maybe<Type>> m_value;

    /** The state of the reactor after the function evaluation. */
    State m_state;

    /** Constructs an uninitialized evaluation. */
    FunctionEvaluation();

    /**
     * Constructs an evaluation resulting in a value and an EVAL.
     * @param value The value returned by the function.
     */
    FunctionEvaluation(Maybe<Type> value);

    /**
     * Constructs an evaluation resulting in a value and an EVAL.
     * @param value The value returned by the function.
     */
    FunctionEvaluation(Type value);

    /**
     * Constructs an evaluation resulting in a value and an EVAL.
     * @param value The value returned by the function.
     */
    FunctionEvaluation(std::optional<Maybe<Type>> value);

    /**
     * Constructs an evaluation resulting in a value and an EVAL.
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
  struct function_reactor_result<FunctionEvaluation<T>> {
    using type = T;
  };

  template<typename T>
  using function_reactor_result_t = typename function_reactor_result<T>::type;

  template<typename T>
  decltype(auto) deref(T& value) noexcept {
    if constexpr(is_reactor_pointer_v<T>) {
      return *value;
    } else {
      return value;
    }
  }

  template<typename T>
  struct FunctionEvaluator {
    template<typename V, typename F, typename P>
    State operator ()(V& value, F& function, const P& pack) const {
      auto evaluation = std::apply(
        [&] (const auto&... arguments) {
          return FunctionEvaluation<T>(function(
            try_call([&] () noexcept(noexcept(deref(arguments).eval())) {
              return deref(arguments).eval();
            })...));
        }, pack);
      if(evaluation.m_value.has_value()) {
        if constexpr(std::is_same_v<V, LocalPtr<T>>) {
          *value = std::move(*evaluation.m_value);
        } else {
          value = std::move(*evaluation.m_value);
        }
      }
      return evaluation.m_state;
    }
  };

  template<>
  struct FunctionEvaluator<void> {
    template<typename V, typename F, typename P>
    State operator ()(V& value, F& function, const P& pack) const {
      std::apply(
        [&] (const auto&... arguments) {
          return FunctionEvaluation<void>(try_call([&] {
            return function(
              try_call([&] () noexcept(noexcept(deref(arguments).eval())) {
                return deref(arguments).eval();
              })...);
          }));
        }, pack);
      return State::EVALUATED;
    }
  };

  template<typename T>
  struct unwrap_local_ptr {
    using type = T;
  };

  template<typename T>
  struct unwrap_local_ptr<LocalPtr<T>> {
    using type = T;
  };

  template<typename T>
  using unwrap_local_ptr_t = typename unwrap_local_ptr<T>::type;

  template<typename F, typename... A>
  struct is_lift_noexcept : std::bool_constant<is_noexcept_function_v<F,
    const unwrap_local_ptr_t<try_maybe_t<reactor_result_t<A>,
    !is_noexcept_reactor_v<A>>>& ...> &&
    std::conjunction_v<is_noexcept_reactor<A>...>> {};

  template<typename F, typename... A>
  constexpr auto is_lift_noexcept_v = is_lift_noexcept<F, A...>::value;
}

  /**
   * A reactor that applies a function to its parameters.
   * @param <F> The type of function to apply.
   * @param <P> The type of arguments to apply the function to.
   */
  template<typename F, typename... A>
  class Lift {
    public:
      using Type = Details::function_reactor_result_t<std::invoke_result_t<F,
        const Maybe<reactor_result_t<A>>&...>>;

      /** The type of function to apply. */
      using Function = F;

      /** A tuple containing the list of arguments to apply to the function. */
      using Arguments = std::tuple<A...>;

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = Details::is_lift_noexcept_v<F, A...>;

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
      bool m_has_continuation;

      State invoke();
  };

  /**
   * Specialization for functions that have no parameters.
   * @param <F> The type of function to apply.
   */
  template<typename F>
  class Lift<F> {
    public:
      using Type = Details::function_reactor_result_t<std::invoke_result_t<F>>;
      using Function = F;
      using Arguments = std::tuple<>;
      static constexpr auto is_noexcept = is_noexcept_function_v<F>;

      /**
       * Constructs a function reactor.
       * @param function The function to apply.
       */
      template<typename FF>
      explicit Lift(FF&& function);

      Lift(const Lift& lift) = default;

      Lift(Lift&& lift) = default;

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const;

      Lift& operator =(const Lift& lift) = default;

      Lift& operator =(Lift&& lift) = default;

    private:
      Function m_function;
      try_maybe_t<Type, std::is_same_v<Type, void> || !is_noexcept> m_value;
      State m_state;
      int m_previous_sequence;

      State invoke();
  };

  template<typename F, typename AF, typename... AR>
  Lift(F&&, AF&&, AR&&...) -> Lift<std::decay_t<F>, to_reactor_t<AF>,
    to_reactor_t<AR>...>;

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

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation()
    : m_state(State::NONE) {}

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(Maybe<Type> value)
    : m_value(std::move(value)),
      m_state(State::EVALUATED) {}

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(Type value)
    : FunctionEvaluation(Maybe(std::move(value))) {}

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(std::optional<Maybe<Type>> value)
      : m_value(std::move(value)) {
    if(m_value.has_value()) {
      m_state = State::EVALUATED;
    } else {
      m_state = State::NONE;
    }
  }

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(std::optional<Type> value)
    : FunctionEvaluation(std::optional<Maybe<Type>>(std::move(value))) {}

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(Maybe<Type> value, State state)
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
  FunctionEvaluation<T>::FunctionEvaluation(Type value, State state)
    : FunctionEvaluation(Maybe(std::move(value)), state) {}

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(std::optional<Maybe<Type>> value,
      State state)
      : m_value(std::move(value)) {
    if(m_value.has_value()) {
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
  FunctionEvaluation<T>::FunctionEvaluation(std::optional<Type> value,
    State state)
    : FunctionEvaluation(std::optional(Maybe(std::move(value))), state) {}

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(State state)
      : m_state(state) {
    assert(!has_evaluation(m_state));
  }

  inline FunctionEvaluation<void>::FunctionEvaluation()
    : m_state(State::NONE) {}

  inline FunctionEvaluation<void>::FunctionEvaluation(Maybe<Type> value)
    : m_value(std::move(value)),
      m_state(State::EVALUATED) {}

  inline FunctionEvaluation<void>::FunctionEvaluation(
      std::optional<Maybe<Type>> value)
      : m_value(std::move(value)) {
    if(m_value.has_value()) {
      m_state = State::EVALUATED;
    } else {
      m_state = State::NONE;
    }
  }

  inline FunctionEvaluation<void>::FunctionEvaluation(Maybe<Type> value,
      State state)
      : m_value(std::move(value)) {
    if(is_complete(state)) {
      m_state = State::COMPLETE_EVALUATED;
    } else {
      m_state = State::EVALUATED;
    }
  }

  inline FunctionEvaluation<void>::FunctionEvaluation(
      std::optional<Maybe<Type>> value, State state)
      : m_value(std::move(value)) {
    if(m_value.has_value()) {
      if(is_complete(state)) {
        m_state = State::COMPLETE_EVALUATED;
      } else {
        m_state = State::EVALUATED;
      }
    } else if(is_complete(state)) {
      m_state = State::COMPLETE;
    } else {
      m_state = State::NONE;
    }
  }

  inline FunctionEvaluation<void>::FunctionEvaluation(State state)
      : m_state(state) {
    assert(!has_evaluation(m_state));
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
      m_previous_sequence(-1),
      m_has_continuation(false) {}

  template<typename F, typename... A>
  Lift<F, A...>::Lift(const Lift& lift)
      : m_function(lift.m_function),
        m_arguments(lift.m_arguments),
        m_handler(std::apply([] (auto&&... arguments) {
          return std::make_tuple(&Details::deref(arguments)...);
        }, m_arguments)),
        m_value(lift.m_value),
        m_state(lift.m_state),
        m_previous_sequence(lift.m_previous_sequence),
        m_has_continuation(lift.m_has_continuation) {
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
        m_previous_sequence(std::move(lift.m_previous_sequence)),
        m_has_continuation(std::move(lift.m_has_continuation)) {
    m_handler.transfer(lift.m_handler);
  }

  template<typename F, typename... A>
  State Lift<F, A...>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    auto state = m_handler.commit(sequence);
    if(has_evaluation(state) || m_has_continuation ||
        is_complete(state) && !is_empty(state) && is_empty(m_state)) {
      m_has_continuation = false;
      auto invocation = invoke();
      if(invocation == State::NONE) {
        if(is_complete(state)) {
          if(is_empty(m_state)) {
            m_state = State::COMPLETE_EMPTY;
          } else {
            m_state = State::COMPLETE;
          }
        } else if(has_continuation(state)) {
          if(is_empty(m_state)) {
            m_state = State::CONTINUE_EMPTY;
          } else {
            m_state = State::CONTINUE;
          }
        } else {
          if(is_empty(m_state)) {
            m_state = State::EMPTY;
          } else {
            m_state = State::NONE;
          }
        }
      } else if(is_complete(invocation)) {
        if(has_evaluation(invocation)) {
          m_state = State::COMPLETE_EVALUATED;
        } else if(is_empty(m_state)) {
          m_state = State::COMPLETE_EMPTY;
        } else {
          m_state = State::COMPLETE;
        }
      } else {
        m_state = invocation;
        m_has_continuation = has_continuation(invocation);
        if(has_continuation(state)) {
          m_state = combine(m_state, State::CONTINUE);
        } else if(is_complete(state) && !m_has_continuation) {
          m_state = combine(m_state, State::COMPLETE);
        }
      }
    } else {
      m_state = state;
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename F, typename... A>
  eval_result_t<typename Lift<F, A...>::Type> Lift<F, A...>::eval() const
      noexcept(is_noexcept){
    return *m_value;
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
    m_has_continuation = lift.m_has_continuation;
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
    m_has_continuation = std::move(lift.m_has_continuation);
    return *this;
  }

  template<typename F, typename... A>
  State Lift<F, A...>::invoke() {
    try {
      return Details::FunctionEvaluator<Type>()(m_value, m_function,
        m_arguments);
    } catch(const std::exception&) {
      if constexpr(!is_noexcept) {
        m_value = std::current_exception();
      }
      return State::EVALUATED;
    }
  }

  template<typename F>
  template<typename FF>
  Lift<F>::Lift(FF&& function)
    : m_function(std::forward<FF>(function)),
      m_state(State::EMPTY),
      m_previous_sequence(-1) {}

  template<typename F>
  State Lift<F>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    auto invocation = invoke();
    if(has_evaluation(invocation)) {
      if(has_continuation(invocation)) {
        m_state = State::CONTINUE_EVALUATED;
      } else {
        m_state = State::COMPLETE_EVALUATED;
      }
    } else if(m_state == State::EMPTY) {
      m_state = State::COMPLETE_EMPTY;
    } else {
      m_state = State::COMPLETE;
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename F>
  eval_result_t<typename Lift<F>::Type> Lift<F>::eval() const {
    return *m_value;
  }

  template<typename F>
  State Lift<F>::invoke() {
    try {
      return Details::FunctionEvaluator<Type>()(m_value, m_function,
        std::tuple<>());
    } catch(const std::exception&) {
      if constexpr(!is_noexcept) {
        m_value = std::current_exception();
      }
      return State::EVALUATED;
    }
  }
}

#endif

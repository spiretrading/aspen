#ifndef ASPEN_LIFT_HPP
#define ASPEN_LIFT_HPP
#include <cassert>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include "Aspen/CommitHandler.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"
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
  decltype(auto) deref(T& value) {
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
            try_call([&] { return deref(arguments).eval(); })...));
        }, pack);
      if(evaluation.m_value.has_value()) {
        value = std::move(*evaluation.m_value);
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
          return FunctionEvaluation<void>(function(
            try_call([&] { return deref(arguments).eval(); })...));
        }, pack);
      return State::EVALUATED;
    }
  };
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

      /**
       * Constructs a function reactor.
       * @param function The function to apply.
       * @param arguments The arguments to apply the <i>function</i> to.
       */
      template<typename FF, typename... AF>
      Lift(FF&& function, AF&&... arguments);

      State commit(int sequence);

      const Type& eval() const;

    private:
      Function m_function;
      Arguments m_arguments;
      CommitHandler m_handler;
      Maybe<Type> m_value;
      State m_state;
      int m_previous_sequence;
      bool m_had_evaluation;

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

      /**
       * Constructs a function reactor.
       * @param function The function to apply.
       */
      template<typename FF>
      Lift(FF&& function);

      State commit(int sequence);

      const Type& eval() const;

    private:
      Function m_function;
      Maybe<Type> m_value;
      State m_state;

      State invoke();
  };

  template<typename F, typename... A>
  Lift(F&&, A&&...) -> Lift<std::decay_t<F>, std::decay_t<A>...>;

  template<typename F>
  Lift(F&&) -> Lift<std::decay_t<F>>;

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
    } else {
      m_state = State::EVALUATED;
    }
  }

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(Type value, State state)
    : FunctionEvaluation(Maybe(std::move(value))) {}

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(std::optional<Maybe<Type>> value,
      State state)
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

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(std::optional<Type> value,
    State state)
    : FunctionEvaluation(std::optional(Maybe(std::move(value))), state) {}

  template<typename T>
  FunctionEvaluation<T>::FunctionEvaluation(State state)
      : m_state(state) {
    assert(!has_evaluation(m_state));
  }

  template<typename F, typename... A>
  template<typename FF, typename... AF>
  Lift<F, A...>::Lift(FF&& function, AF&&... arguments)
    : m_function(std::forward<FF>(function)),
      m_arguments(std::forward<AF>(arguments)...),
      m_handler(
        [&] {
          auto children = std::vector<Box<void>>();
          std::apply([&] (auto&... arguments) {
            (children.emplace_back(&Details::deref(arguments)), ...);
          }, m_arguments);
          return children;
        }()),
      m_state(State::UNINITIALIZED),
      m_previous_sequence(-1),
      m_had_evaluation(false) {}

  template<typename F, typename... A>
  State Lift<F, A...>::commit(int sequence) {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    m_state = m_handler.commit(sequence);
    if(has_evaluation(m_state)) {
      auto invocation = invoke();
      if(invocation == State::NONE) {
        if(is_complete(m_state)) {
          if(m_had_evaluation) {
            m_state = State::COMPLETE_EVALUATED;
          } else {
            m_state = State::COMPLETE_EMPTY;
          }
        } else {
          m_state = State::NONE;
        }
      } else if(is_complete(invocation)) {
        if(m_had_evaluation || has_evaluation(invocation)) {
          m_state = State::COMPLETE_EVALUATED;
        } else {
          m_state = State::COMPLETE_EMPTY;
        }
      }
    }
    m_previous_sequence = sequence;
    m_had_evaluation |= has_evaluation(m_state);
    return m_state;
  }

  template<typename F, typename... A>
  const typename Lift<F, A...>::Type& Lift<F, A...>::eval() const {
    return m_value.get();
  }

  template<typename F, typename... A>
  State Lift<F, A...>::invoke() {
    try {
      return Details::FunctionEvaluator<Type>()(m_value, m_function,
        m_arguments);
    } catch(const std::exception&) {
      m_value = std::current_exception();
      return State::EVALUATED;
    }
  }

  template<typename F>
  template<typename FF>
  Lift<F>::Lift(FF&& function)
    : m_function(std::forward<FF>(function)),
      m_state(State::UNINITIALIZED) {}

  template<typename F>
  State Lift<F>::commit(int sequence) {
    if(m_state != State::UNINITIALIZED) {
      return m_state;
    }
    auto invocation = invoke();
    if(has_evaluation(invocation)) {
      m_state = State::COMPLETE_EVALUATED;
    } else {
      m_state = State::COMPLETE_EMPTY;
    }
    return m_state;
  }

  template<typename F>
  const typename Lift<F>::Type& Lift<F>::eval() const {
    return m_value.get();
  }

  template<typename F>
  State Lift<F>::invoke() {
    try {
      return Details::FunctionEvaluator<Type>()(m_value, m_function,
        std::tuple<>());
    } catch(const std::exception&) {
      m_value = std::current_exception();
      return State::EVALUATED;
    }
  }
}

#endif

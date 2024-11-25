export module Aspen:Fold;

import std;
import :Lift;
import :Maybe;
import :Shared;
import :State;
import :Traits;

export namespace Aspen {

  /**
   * A placeholder used to provide a single argument to a fold reactor.
   * @param <T> The type to evaluate to.
   */
  template<typename T>
  class FoldArgument {
    public:
      using Type = T;

      /** Constructs a FoldArgument. */
      FoldArgument() = default;

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const;

    private:
      template<typename, typename> friend class Fold;
      Maybe<Type> m_value;
      std::optional<Maybe<Type>> m_next_value;

      void update(Maybe<Type> value);
  };

  /**
   * A reactor that repeatedly applies a binary reactor over a series.
   * @param <E> The type of reactor to apply to the series.
   * @param <S> The type of reactor producing the series.
   */
  template<typename E, typename S>
  class Fold {
    public:
      using Type = reactor_result_t<E>;

      /**
       * Constructs a Fold.
       * @param evaluator The reactor used to evaluate/update the series of
       *        values.
       * @param left The reactor used as the left-hand/first argument to the
       *        evaluator.
       * @param right The reactor used as the right-hand/second argument to the
       *        evaluator.
       * @param series The reactor producing the series.
       */
      template<typename EF, typename SF>
      Fold(EF&& evaluator, Shared<FoldArgument<Type>> left,
        Shared<FoldArgument<Type>> right, SF&& series);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const;

    private:
      E m_evaluator;
      Shared<FoldArgument<Type>> m_left;
      Shared<FoldArgument<Type>> m_right;
      S m_series;
      Maybe<Type> m_value;
      std::optional<Maybe<Type>> m_previous_value;
  };

  template<typename E, typename S>
  Fold(E&&, Shared<FoldArgument<reactor_result_t<E>>>,
    Shared<FoldArgument<reactor_result_t<E>>>, S&&) ->
    Fold<to_reactor_t<E>, to_reactor_t<S>>;

  /** Returns a new FoldArgument. */
  template<typename T>
  auto make_fold_argument() {
    return Shared<FoldArgument<T>>();
  }

  /**
   * Builds a Fold reactor.
   * @param evaluator The reactor used to evaluate/update the series of
   *        values.
   * @param left The reactor used as the left-hand/first argument to the
   *        evaluator.
   * @param right The reactor used as the right-hand/second argument to the
   *        evaluator.
   * @param series The reactor producing the series.
   */
  template<typename E, typename S>
  auto fold(E&& evaluator, Shared<FoldArgument<reactor_result_t<E>>> left,
      Shared<FoldArgument<reactor_result_t<E>>> right, S&& series) {
    return Fold(std::forward<E>(evaluator), std::move(left), std::move(right),
      std::forward<S>(series));
  }

  /**
   * Builds a Fold reactor by directly lifting a function.
   * @param f The function used to fold the series.
   * @param series The reactor producing the series to fold.
   */
  template<typename F, typename S>
  auto fold(F&& f, S&& series) {
    using SeriesType = reactor_result_t<S>;
    using Type = std::decay_t<decltype(
      std::declval<F>()(std::declval<SeriesType>(),
      std::declval<SeriesType>()))>;
    auto left = make_fold_argument<SeriesType>();
    auto right = make_fold_argument<SeriesType>();
    return Fold(Lift(std::forward<F>(f), left, right), left, right,
      std::forward<S>(series));
  }

  template<typename T>
  State FoldArgument<T>::commit(int sequence) noexcept {
    if(m_next_value.has_value()) {
      m_value = std::move(*m_next_value);
      m_next_value = std::nullopt;
      return State::EVALUATED;
    }
    return State::NONE;
  }

  template<typename T>
  eval_result_t<typename FoldArgument<T>::Type> FoldArgument<T>::eval() const {
    return m_value;
  }

  template<typename T>
  void FoldArgument<T>::update(Maybe<Type> value) {
    m_next_value.emplace(std::move(value));
  }

  template<typename E, typename S>
  template<typename EF, typename SF>
  Fold<E, S>::Fold(EF&& evaluator, Shared<FoldArgument<Type>> left,
      Shared<FoldArgument<Type>> right, SF&& series)
    : m_evaluator(std::forward<EF>(evaluator)),
      m_left(std::move(left)),
      m_right(std::move(right)),
      m_series(std::forward<SF>(series)) {}

  template<typename E, typename S>
  State Fold<E, S>::commit(int sequence) noexcept {
    auto state = State::NONE;
    auto series_state = m_series.commit(sequence);
    if(has_evaluation(series_state)) {
      if(!m_previous_value.has_value()) {
        if(!is_complete(series_state)) {
          m_previous_value = try_call([&] { return m_series.eval(); });
        }
      } else {
        m_left->update(std::move(*m_previous_value));
        m_right->update(try_call([&] { return m_series.eval(); }));
        state = m_evaluator.commit(sequence);
        if(has_evaluation(state)) {
          m_value = try_call([&] { return m_evaluator.eval(); });
          m_previous_value = m_value;
        }
      }
    }
    if(has_continuation(series_state)) {
      state = combine(state, State::CONTINUE);
    } else if(is_complete(series_state)) {
      state = combine(state, State::COMPLETE);
    }
    return state;
  }

  template<typename E, typename S>
  eval_result_t<typename Fold<E, S>::Type> Fold<E, S>::eval() const {
    return m_value;
  }
}

#ifndef ASPEN_FOLD_HPP
#define ASPEN_FOLD_HPP
#include <memory>
#include <optional>
#include <utility>
#include "Aspen/Lift.hpp"
#include "Aspen/LocalPtr.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * A placeholder used to provide a single argument to a fold reactor.
   * @param <T> The type to evaluate to.
   */
  template<typename T>
  class FoldArgument {
    public:
      using Type = T;

      /** Constructs a FoldArgument. */
      FoldArgument() noexcept;

      State commit(int sequence) noexcept;

      const Type& eval() const;

    private:
      template<typename, typename> friend class Fold;
      State m_state;
      Maybe<Type> m_value;
      std::optional<Maybe<Type>> m_next_value;
      int m_previous_sequence;

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
      Fold(EF&& evaluator, std::shared_ptr<FoldArgument<Type>> left,
        std::shared_ptr<FoldArgument<Type>> right, SF&& series);

      State commit(int sequence) noexcept;

      const Type& eval() const;

    private:
      try_ptr_t<E> m_evaluator;
      std::shared_ptr<FoldArgument<Type>> m_left;
      std::shared_ptr<FoldArgument<Type>> m_right;
      try_ptr_t<S> m_series;
      Maybe<Type> m_value;
      std::optional<Maybe<Type>> m_previous_value;
      State m_state;
      int m_previous_sequence;
  };

  template<typename EF, typename SF>
  Fold(EF&&, std::shared_ptr<FoldArgument<reactor_result_t<EF>>>,
    std::shared_ptr<FoldArgument<reactor_result_t<EF>>>, SF&&) ->
    Fold<std::decay_t<EF>, std::decay_t<SF>>;

  /** Returns a new FoldArgument. */
  template<typename T>
  auto make_fold_argument() {
    return std::make_shared<FoldArgument<T>>();
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
  auto fold(E&& evaluator,
      std::shared_ptr<FoldArgument<reactor_result_t<E>>> left,
      std::shared_ptr<FoldArgument<reactor_result_t<E>>> right, S&& series) {
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
  FoldArgument<T>::FoldArgument() noexcept
    : m_state(State::NONE),
      m_previous_sequence(-1) {}

  template<typename T>
  State FoldArgument<T>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence) {
      return m_state;
    } else if(m_next_value.has_value()) {
      m_value = std::move(*m_next_value);
      m_next_value = std::nullopt;
      m_state = State::EVALUATED;
    } else {
      m_state = State::NONE;
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename T>
  const typename FoldArgument<T>::Type& FoldArgument<T>::eval() const {
    return m_value;
  }

  template<typename T>
  void FoldArgument<T>::update(Maybe<Type> value) {
    m_next_value.emplace(std::move(value));
  }

  template<typename E, typename S>
  template<typename EF, typename SF>
  Fold<E, S>::Fold(EF&& evaluator, std::shared_ptr<FoldArgument<Type>> left,
      std::shared_ptr<FoldArgument<Type>> right, SF&& series)
    : m_evaluator(std::forward<EF>(evaluator)),
      m_left(std::move(left)),
      m_right(std::move(right)),
      m_series(std::forward<SF>(series)),
      m_state(State::NONE),
      m_previous_sequence(-1) {}

  template<typename E, typename S>
  State Fold<E, S>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    auto series_state = m_series->commit(sequence);
    if(series_state == State::NONE || series_state == State::CONTINUE ||
        series_state == State::COMPLETE_EMPTY) {
      m_state = series_state;
    } else if(has_evaluation(series_state) ||
        is_complete(series_state) && !is_empty(series_state)) {
      if(!m_previous_value.has_value()) {
        if(is_complete(series_state)) {
          m_state = State::COMPLETE_EMPTY;
        } else {
          m_previous_value = try_call([&] { return m_series->eval(); });
          m_state = State::NONE;
        }
      } else {
        m_left->update(std::move(*m_previous_value));
        m_right->update(try_call([&] { return m_series->eval(); }));
        m_state = m_evaluator->commit(sequence);
        if(has_evaluation(m_state)) {
          m_value = try_call([&] { return m_evaluator->eval(); });
          m_previous_value = m_value;
        }
      }
    }
    if(has_continuation(series_state)) {
      m_state = combine(m_state, State::CONTINUE);
    } else if(is_complete(series_state)) {
      m_state = combine(m_state, State::COMPLETE);
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename E, typename S>
  const typename Fold<E, S>::Type& Fold<E, S>::eval() const {
    return m_value;
  }
}

#endif

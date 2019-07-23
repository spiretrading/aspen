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
      FoldArgument();

      State commit(int sequence);

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

      State commit(int sequence);

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

  template<typename T>
  FoldArgument<T>::FoldArgument()
    : m_state(State::NONE),
      m_previous_sequence(-1) {}

  template<typename T>
  State FoldArgument<T>::commit(int sequence) {
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
  State Fold<E, S>::commit(int sequence) {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    auto series_state = m_series->commit(sequence);
    if(series_state == State::NONE) {
      m_state = State::NONE;
    } else if(has_evaluation(series_state)) {
      if(!m_previous_value.has_value()) {
        m_previous_value = try_call([&] { return m_series.eval(); });
        m_update = BaseReactor::Update::NONE;
        return BaseReactor::Update::NONE;
      }
      m_leftChild->Set(std::move(*m_previousValue), sequenceNumber);
      m_rightChild->Set(TryEval(*m_producer), sequenceNumber);
      m_update = m_evaluation->Commit(sequenceNumber);
      if(HasEval(m_update)) {
        m_value = TryEval(*m_evaluation);
        m_previousValue = m_value;
      }


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

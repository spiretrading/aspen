#ifndef ASPEN_STATE_HPP
#define ASPEN_STATE_HPP

namespace Aspen {

  //! Lists the states of a reactor after a commit operation.
  enum class State {

    //! No update.
    NONE = 0,

    //! The reactor has never produced an evaluation.
    EMPTY = 1,

    //! The reactor has a new value.
    EVALUATED = 2,

    //! The reactor should immediately be committed again.
    CONTINUE = 4,

    //! The reactor has a new value and should immediately be committed again.
    CONTINUE_EVALUTED = EVALUATED | CONTINUE,

    //! The reactor has come to an end.
    COMPLETE = 8,

    //! The reactor has terminated without ever producing an evaluation.
    COMPLETE_EMPTY = COMPLETE | EMPTY,

    //! The reactor has terminated with an evaluation.
    COMPLETE_EVALUATED = COMPLETE | EVALUATED
  };

  /** Returns <code>true</code> iff a reactor State is in an EVALUATED state. */
  inline constexpr bool has_evaluation(State state) {
    return (static_cast<int>(state) & static_cast<int>(State::EVALUATED)) != 0;
  }

  /** Returns <code>true</code> iff a reactor State is in an EMPTY state. */
  inline constexpr bool is_empty(State state) {
    return (static_cast<int>(state) & static_cast<int>(State::EMPTY)) != 0;
  }

  /** Returns <code>true</code> iff a reactor State is in a CONTINUE state. */
  inline constexpr bool has_continuation(State state) {
    return (static_cast<int>(state) & static_cast<int>(State::CONTINUE)) != 0;
  }

  /** Returns <code>true</code> iff a reactor State is in a COMPLETE state. */
  inline constexpr bool is_complete(State state) {
    return (static_cast<int>(state) & static_cast<int>(State::COMPLETE)) != 0;
  }
}

#endif

#ifndef ASPEN_STATE_HPP
#define ASPEN_STATE_HPP

namespace Aspen {

  //! Lists the states of a reactor after a commit operation.
  enum class State : unsigned char {

    //! No update.
    NONE = 0,

    //! The reactor has never produced an evaluation.
    EMPTY = 1,

    //! The reactor has a new value.
    EVALUATED = 2,

    //! The reactor should immediately be committed again.
    CONTINUE = 4,

    //! The reactor is empty and should immediately be committed again.
    CONTINUE_EMPTY = EMPTY | CONTINUE,

    //! The reactor has a new value and should immediately be committed again.
    CONTINUE_EVALUATED = EVALUATED | CONTINUE,

    //! The reactor has come to an end.
    COMPLETE = 8,

    //! The reactor has terminated without ever producing an evaluation.
    COMPLETE_EMPTY = COMPLETE | EMPTY,

    //! The reactor has terminated with an evaluation.
    COMPLETE_EVALUATED = COMPLETE | EVALUATED
  };

  /** Returns the combination of two States. */
  constexpr State combine(State a, State b) {
    return static_cast<State>(
      static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
  }

  /** Returns <code>true</code> iff a reactor State is in an EVALUATED state. */
  constexpr bool has_evaluation(State state) {
    return (static_cast<unsigned char>(state) & static_cast<unsigned char>(
      State::EVALUATED)) != 0;
  }

  /** Returns <code>true</code> iff a reactor State is in an EMPTY state. */
  constexpr bool is_empty(State state) {
    return (static_cast<unsigned char>(state) &
      static_cast<unsigned char>(State::EMPTY)) != 0;
  }

  /** Returns <code>true</code> iff a reactor State is in a CONTINUE state. */
  constexpr bool has_continuation(State state) {
    return (static_cast<unsigned char>(state) &
      static_cast<unsigned char>(State::CONTINUE)) != 0;
  }

  /** Returns <code>true</code> iff a reactor State is in a COMPLETE state. */
  constexpr bool is_complete(State state) {
    return (static_cast<unsigned char>(state) &
      static_cast<unsigned char>(State::COMPLETE)) != 0;
  }

  /** Sets a state flag. */
  constexpr State set(State state, State update) {
    return static_cast<State>(static_cast<unsigned char>(state) |
      static_cast<unsigned char>(update));
  }

  /** Resets a state flag. */
  constexpr State reset(State state, State update) {
    return static_cast<State>(static_cast<unsigned char>(state) &
      ~static_cast<unsigned char>(update));
  }
}

#endif

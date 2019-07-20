#ifndef ASPEN_STATE_HPP
#define ASPEN_STATE_HPP

namespace Aspen {

  //! Lists the states of a reactor after a commit operation.
  enum class State {

    //! Reactor is uninitialized.
    UNINITIALIZED = 0,

    //! No update.
    NONE = 1,

    //! The reactor has a new value.
    EVALUATED = 2,

    //! The reactor has come to an end.
    COMPLETE = 4,

    //! The reactor has terminated without an evaluation.
    COMPLETE_EMPTY = COMPLETE | NONE,

    //! The reactor has terminated with an evaluation.
    COMPLETE_EVALUATED = COMPLETE | EVALUATED
  };

  /** Returns <code>true</code> iff a reactor's state  is complete. */
  inline constexpr bool is_complete(State state) {
    return (static_cast<int>(state) & static_cast<int>(State::COMPLETE)) != 0;
  }
}

#endif

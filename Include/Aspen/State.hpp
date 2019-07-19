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

  /**
   * Combines two State values together.
   * @param lhs The State to modify.
   * @param rhs The State to combine with lhs.
   * @return <i>lhs</i>.
   */
  inline constexpr State combine(State& lhs, State rhs) {
    lhs = static_cast<State>(static_cast<int>(lhs) | static_cast<int>(rhs));
    return lhs;
  }

  /** Returns <code>true</code> iff a reactor's state  is complete. */
  inline constexpr bool is_complete(State state) {
    return (static_cast<int>(state) & static_cast<int>(State::COMPLETE)) != 0;
  }
}

#endif

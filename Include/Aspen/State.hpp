#ifndef ASPEN_STATE_HPP
#define ASPEN_STATE_HPP
#include <cassert>
#include <ostream>

namespace Aspen {

  /** Lists the states of a reactor after a commit operation. */
  enum class State : unsigned char {

    /** No update. */
    NONE = 0,

    /** The reactor has a new value. */
    EVALUATED = 1,

    /** The reactor should immediately be committed again. */
    CONTINUE = 2,

    /** The reactor has come to an end. */
    COMPLETE = 4,

    /**
     * The reactor has a new value and should immediately be committed again.
     */
    CONTINUE_EVALUATED = EVALUATED | CONTINUE,

    /** The reactor has terminated with an evaluation. */
    COMPLETE_EVALUATED = COMPLETE | EVALUATED
  };

  /** Returns <code>true</code> iff a reactor State is in an EVALUATED state. */
  constexpr bool has_evaluation(State state) {
    return (static_cast<unsigned char>(state) &
      static_cast<unsigned char>(State::EVALUATED)) != 0;
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

  /** Returns the combination of two States. */
  constexpr State combine(State left, State right) {
    auto state = static_cast<State>(
      static_cast<unsigned char>(left) | static_cast<unsigned char>(right));
    assert(!is_complete(state) || !has_continuation(state));
    return state;
  }

  /**
   * Clears a set of flags from a state.
   * @param state The state to clear the flags from.
   * @param flags The flags to clear.
   * @return The <i>state</i> with the <i>flags</i> cleared.
   */
  constexpr State reset(State state, State flags) {
    return static_cast<State>(static_cast<unsigned char>(state) &
      ~static_cast<unsigned char>(flags));
  }

  inline std::ostream& operator <<(std::ostream& sink, State state) {
    if(state == State::NONE) {
      return sink << "NONE";
    }
    auto separator = "";
    if(is_complete(state)) {
      sink << "COMPLETE";
      separator = "_";
    }
    if(has_continuation(state)) {
      sink << separator << "CONTINUE";
      separator = "_";
    }
    if(has_evaluation(state)) {
      sink << separator << "EVALUATED";
    }
    return sink;
  }
}

#endif

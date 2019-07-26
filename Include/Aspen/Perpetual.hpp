#ifndef ASPEN_PERPETUAL_HPP
#define ASPEN_PERPETUAL_HPP
#include "Aspen/State.hpp"

namespace Aspen {

  /**
   * A reactor that always evaluates to nothing, useful for building up
   * reactors that need to iterate.
   */
  class Perpetual {
    public:
      using Type = void;

      constexpr State commit(int sequence) noexcept;

      constexpr void eval() const noexcept;
  };

  /** Returns a reactor that perpetually evaluates. */
  constexpr auto perpetual() noexcept {
    return Perpetual();
  }

  constexpr State Perpetual::commit(int sequence) noexcept {
    return State::CONTINUE_EVALUATED;
  }

  constexpr void Perpetual::eval() const noexcept {}
}

#endif

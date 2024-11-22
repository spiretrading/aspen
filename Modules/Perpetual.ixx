export module Aspen:Perpetual;

import :State;

export namespace Aspen {

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

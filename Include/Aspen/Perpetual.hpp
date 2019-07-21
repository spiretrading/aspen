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

      constexpr State commit(int sequence);

      constexpr void eval() const;
  };

  inline constexpr State Perpetual::commit(int sequence) {
    return State::EVALUATED;
  }

  inline constexpr void Perpetual::eval() const {}
}

#endif

#ifndef ASPEN_PYTHON_GIL_ACQUIRE_REACTOR_HPP
#define ASPEN_PYTHON_GIL_ACQUIRE_REACTOR_HPP
#include <cstdint>
#include <pybind11/pybind11.h>
#include <type_traits>
#include "Aspen/LocalPtr.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Wraps a reactor ensuring that the Python GIL is acquired before performing
   * any operation.
   */
  template<typename R>
  class GilAcquireReactor {
    public:
      using Type = reactor_result_t<R>;

      /** The type returned by an evaluation. */
      using Result = reactor_evaluation_t<R>;

      /**
       * Constructs a GilAcquireReactor.
       * @param reactor Initializes the reactor to wrap.
       */
      template<typename RF>
      GilAcquireReactor(RF&& reactor);

      State commit(std::uint64_t sequence) noexcept;

      Result eval() const noexcept(is_noexcept_reactor_v<R>);

    private:
      try_ptr_t<R> m_reactor;
  };

  template<typename R>
  GilAcquireReactor(R&&) -> GilAcquireReactor<std::decay_t<R>>;

  template<typename R>
  template<typename RF>
  GilAcquireReactor<R>::GilAcquireReactor(RF&& reactor)
    : m_reactor(std::forward<RF>(reactor)) {}

  template<typename R>
  State GilAcquireReactor<R>::commit(std::uint64_t sequence) noexcept {
    auto lock = pybind11::gil_scoped_acquire();
    return m_reactor->commit(sequence);
  }

  template<typename R>
  typename GilAcquireReactor<R>::Result GilAcquireReactor<R>::eval() const
      noexcept(is_noexcept_reactor_v<R>) {
    auto lock = pybind11::gil_scoped_acquire();
    return m_reactor->eval();
  }
}

#endif

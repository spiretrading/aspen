export module Aspen:GilAcquireReactor;

import <type_traits>;
import <pybind11/pybind11.h>;
import :LocalPtr;
import :Traits;

export namespace Aspen {

  /**
   * Wraps a reactor ensuring that the Python GIL is acquired before performing
   * any operation.
   */
  template<typename R>
  class GilAcquireReactor {
    public:
      using Type = reactor_result_t<R>;

      /**
       * Constructs a GilAcquireReactor.
       * @param reactor Initializes the reactor to wrap.
       */
      template<typename RF>
      GilAcquireReactor(RF&& reactor);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept_reactor_v<R>);

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
  State GilAcquireReactor<R>::commit(int sequence) noexcept {
    auto lock = pybind11::gil_scoped_acquire();
    return m_reactor->commit(sequence);
  }

  template<typename R>
  eval_result_t<typename GilAcquireReactor<R>::Type>
      GilAcquireReactor<R>::eval() const noexcept(is_noexcept_reactor_v<R>) {
    auto lock = pybind11::gil_scoped_acquire();
    return m_reactor->eval();
  }
}

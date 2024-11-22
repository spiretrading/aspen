export module Aspen:PythonBox;

import <utility>;
import <pybind11/pybind11.h>;
import :State;
import :Traits;

export namespace Aspen {

  /**
   * Wraps a Python object implementing a reactor into a box.
   * @param <T> The type of value to evaluate to.
   */
  template<typename T>
  class PythonBox {
    public:
      using Type = T;

      /**
       * Constructs a PythonBox from a Python object.
       * @param reactor The Python object implementing the reactor to box.
       */
      explicit PythonBox(pybind11::object reactor);

      State commit(int sequence) noexcept;

      Type eval() const;

    private:
      pybind11::object m_reactor;
  };

  template<typename T>
  PythonBox<T>::PythonBox(pybind11::object reactor)
    : m_reactor(std::move(reactor)) {}

  template<typename T>
  State PythonBox<T>::commit(int sequence) noexcept {
    return m_reactor.attr("commit")(sequence).template cast<State>();
  }

  template<typename T>
  typename PythonBox<T>::Type PythonBox<T>::eval() const {
    return m_reactor.attr("eval")().template cast<T>();
  }
}

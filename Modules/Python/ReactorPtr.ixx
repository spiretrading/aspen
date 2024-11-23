module;
#include <pybind11/pybind11.h>

export module Aspen.Python:ReactorPtr;

import <memory>;
import <utility>;
import Aspen;

export namespace Aspen {

  /**
   * The type used to ensure that all reactors are properly shared from within
   * Python.
   */
  template<typename R>
  class ReactorPtr {
    public:
      using Reactor = R;
      using Result = decltype(std::declval<R>().eval());
      static constexpr auto is_noexcept = is_noexcept_reactor_v<Reactor>;

      /** Constructs a pointer to a null object. */
      ReactorPtr() = default;

      /** Constructs a pointer to a reactor. */
      ReactorPtr(Reactor* reactor);

      /** Returns a reference to the reactor. */
      const Shared<Unique<Reactor>>& operator *() const noexcept;

      /** Returns a pointer to the reactor. */
      const Shared<Unique<Reactor>>* operator ->() const noexcept;

      /** Returns a reference to the reactor. */
      Shared<Unique<Reactor>>& operator *() noexcept;

      /** Returns a pointer to the reactor. */
      Shared<Unique<Reactor>>* operator ->() noexcept;

      State commit(int sequence) noexcept;

      Result eval() const noexcept(is_noexcept);

    private:
      std::shared_ptr<Shared<Unique<Reactor>>> m_reactor;
  };

  template<typename R>
  ReactorPtr<R>::ReactorPtr(Reactor* reactor)
    : m_reactor(std::make_shared<Shared<Unique<Reactor>>>(reactor)) {}

  template<typename R>
  const Shared<Unique<typename ReactorPtr<R>::Reactor>>&
      ReactorPtr<R>::operator *() const noexcept {
    return *m_reactor;
  }

  template<typename R>
  const Shared<Unique<typename ReactorPtr<R>::Reactor>>*
      ReactorPtr<R>::operator ->() const noexcept {
    return &*m_reactor;
  }

  template<typename R>
  Shared<Unique<typename ReactorPtr<R>::Reactor>>&
      ReactorPtr<R>::operator *() noexcept {
    return *m_reactor;
  }

  template<typename R>
  Shared<Unique<typename ReactorPtr<R>::Reactor>>*
      ReactorPtr<R>::operator ->() noexcept {
    return &*m_reactor;
  }

  template<typename R>
  State ReactorPtr<R>::commit(int sequence) noexcept {
    return m_reactor->commit(sequence);
  }

  template<typename R>
  typename ReactorPtr<R>::Result ReactorPtr<R>::eval()
      const noexcept(is_noexcept) {
    return m_reactor->eval();
  }
}

export {
  PYBIND11_DECLARE_HOLDER_TYPE(T, Aspen::ReactorPtr<T>);

  namespace pybind11::detail {
    template<typename T>
    struct holder_helper<Aspen::ReactorPtr<T>> {
      static const T* get(const Aspen::ReactorPtr<T>& p) {
        return &***p;
      }
    };
  }
}

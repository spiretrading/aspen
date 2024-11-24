module;
#include <exception>

export module Aspen:Sync;

import :State;
import :Traits;

export namespace Aspen {
namespace Details {
  struct Empty {};
  struct Exception {
    std::exception_ptr m_exception;
  };
}

  /** Used to keep a value synchronized with a reactor.
      @param <R> The type of reactor producing values.
      @param <V> The type of the value to synchronize with the reactor.
   */
  template<typename R, typename V = reactor_result_t<R>>
  class Sync : private std::conditional_t<is_noexcept_reactor_v<R>,
      Details::Empty, Details::Exception> {
    public:
      using Type = V;

      /** The type of reactor producing values. */
      using Reactor = R;

      static constexpr auto is_noexcept = is_noexcept_reactor_v<R>;

      /** Constructs a Sync.
          @param value The value to keep synchronized with the <i>reactor</i>.
          @param reactor The reactor producing the values.
       */
      Sync(Type& value, Reactor reactor);

      State commit(int sequence) noexcept;

      const Type& eval() const noexcept(is_noexcept);

    private:
      Type* m_value;
      Reactor m_reactor;
  };

  template<typename R, typename V>
  Sync<R, V>::Sync(Type& value, Reactor reactor)
    : m_value(&value),
      m_reactor(std::move(reactor)) {}

  template<typename R, typename V>
  State Sync<R, V>::commit(int sequence) noexcept {
    auto state = m_reactor.commit(sequence);
    if(has_evaluation(state)) {
      if constexpr(is_noexcept) {
        *m_value = m_reactor.eval();
      } else {
        try {
          *m_value = m_reactor.eval();
          this->m_exception = nullptr;
        } catch(...) {
          this->m_exception = std::current_exception();
        }
      }
    }
    return state;
  }

  template<typename R, typename V>
  const typename Sync<R, V>::Type& Sync<R, V>::eval() const
      noexcept(is_noexcept) {
    if constexpr(!is_noexcept) {
      if(this->m_exception) {
        std::rethrow_exception(this->m_exception);
      }
    }
    return *m_value;
  }
}

#ifndef ASPEN_SYNC_HPP
#define ASPEN_SYNC_HPP
#include <cstdint>
#include <exception>
#include <type_traits>
#include <utility>
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {
namespace Details {
  struct Empty {};

  struct Exception {
    std::exception_ptr m_exception;
  };
}

  /**
   * Used to keep a value synchronized with a reactor.
   * @param <R> The type of reactor producing values.
   * @param <V> The type of the value to synchronize with the reactor.
   */
  template<IsReactor R, typename V = reactor_result_t<R>> requires
    std::is_assignable_v<V&, decltype(std::declval<const R&>().eval())>
  class Sync : private std::conditional_t<is_noexcept_reactor_v<R>,
      Details::Empty, Details::Exception> {
    public:

      /** The type of the value to synchronize with the reactor. */
      using Type = V;

      /** The type of reactor producing values. */
      using Reactor = R;

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_reactor_v<R>;

      /**
       * Constructs a Sync.
       * @param value The value to keep synchronized with the <i>reactor</i>,
       *        which must outlive this reactor.
       * @param reactor The reactor producing the values.
       */
      Sync(Type& value, Reactor reactor);

      /** Returns the exception thrown by the reactor, or nullptr for none. */
      std::exception_ptr get_exception() const noexcept;

      State commit(std::uint64_t sequence) noexcept;
      const Type& eval() const noexcept(is_noexcept);

    private:
      Type* m_value;
      Reactor m_reactor;
  };

  template<IsReactor R, typename V> requires
    std::is_assignable_v<V&, decltype(std::declval<const R&>().eval())>
  Sync<R, V>::Sync(Type& value, Reactor reactor)
    : m_value(&value),
      m_reactor(std::move(reactor)) {}

  template<IsReactor R, typename V> requires
    std::is_assignable_v<V&, decltype(std::declval<const R&>().eval())>
  std::exception_ptr Sync<R, V>::get_exception() const noexcept {
    if constexpr(is_noexcept) {
      return nullptr;
    } else {
      return this->m_exception;
    }
  }

  template<IsReactor R, typename V> requires
    std::is_assignable_v<V&, decltype(std::declval<const R&>().eval())>
  State Sync<R, V>::commit(std::uint64_t sequence) noexcept {
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

  template<IsReactor R, typename V> requires
    std::is_assignable_v<V&, decltype(std::declval<const R&>().eval())>
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

#endif

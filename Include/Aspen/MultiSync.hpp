#ifndef ASPEN_MULTI_SYNC_HPP
#define ASPEN_MULTI_SYNC_HPP
#include <cstdint>
#include <exception>
#include <type_traits>
#include <utility>
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/StaticCommitHandler.hpp"
#include "Aspen/Sync.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Used to keep a series of fields synchronized with a corresponding series
   * of reactors.
   * @param <V> The value to synchronize.
   * @param <R> The series of reactors used to synchronize the value.
   */
  template<typename V, IsReactor... R>
  class MultiSync : private std::conditional_t<
      is_noexcept_reactor_v<R...>, Details::Empty, Details::Exception> {
    public:

      /** The type of the value being synchronized. */
      using Type = V;

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_reactor_v<R...>;

      /**
       * Constructs a MultiSync.
       * @param value The value to keep synchronized, which must outlive this
       *        reactor.
       * @param reactors The reactors used to synchronize the <i>value</i>.
       */
      explicit MultiSync(Type& value, R... reactors);

      /** Returns the exception thrown by a reactor, or nullptr for none. */
      std::exception_ptr get_exception() const noexcept;

      State commit(std::uint64_t sequence) noexcept;
      const Type& eval() const noexcept(is_noexcept);

    private:
      Type* m_value;
      StaticCommitHandler<R...> m_reactors;
  };

  template<typename V, IsReactor... R>
  MultiSync<V, R...>::MultiSync(Type& value, R... reactors)
    : m_value(&value),
      m_reactors(std::move(reactors)...) {}

  template<typename V, IsReactor... R>
  std::exception_ptr MultiSync<V, R...>::get_exception() const noexcept {
    if constexpr(is_noexcept) {
      return nullptr;
    } else {
      return this->m_exception;
    }
  }

  template<typename V, IsReactor... R>
  State MultiSync<V, R...>::commit(std::uint64_t sequence) noexcept {
    auto state = m_reactors.commit(sequence);
    if constexpr(!is_noexcept) {
      if(has_evaluation(state)) {
        this->m_exception = apply([] (const auto&... reactors) {
          auto exception = std::exception_ptr();
          auto find = [&] (const auto& reactor) {
            if(!exception) {
              exception = Details::get_exception(reactor);
            }
          };
          (find(reactors), ...);
          return exception;
        }, m_reactors);
      }
    }
    return state;
  }

  template<typename V, IsReactor... R>
  const typename MultiSync<V, R...>::Type& MultiSync<V, R...>::eval()
      const noexcept(is_noexcept) {
    if constexpr(!is_noexcept) {
      if(this->m_exception) {
        std::rethrow_exception(this->m_exception);
      }
    }
    return *m_value;
  }
}

#endif

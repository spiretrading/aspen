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
  State MultiSync<V, R...>::commit(std::uint64_t sequence) noexcept {
    auto state = m_reactors.commit(sequence);
    if constexpr(!is_noexcept) {
      if(has_evaluation(state)) {
        try {
          apply([] (const auto&... reactors) {
            (reactors.eval(), ...);
          }, m_reactors);
          this->m_exception = nullptr;
        } catch(...) {
          this->m_exception = std::current_exception();
        }
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

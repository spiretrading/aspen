#ifndef ASPEN_MULTI_SYNC_HPP
#define ASPEN_MULTI_SYNC_HPP
#include "Aspen/State.hpp"
#include "Aspen/StaticCommitHandler.hpp"
#include "Aspen/Sync.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {
namespace Details {
  template<typename... T>
  constexpr bool test_all(T... args) {
    return (args && ...);
  }
}

  /** Used to keep a series of fields synchronized with a corresponding series
      of reactors.
      @param <V> The value to synchronize.
      @param <R> The series of reactors used to synchronize the value.
   */
  template<typename V, typename... R>
  class MultiSync : private std::conditional_t<
      Details::test_all(is_noexcept_reactor_v<R>...), Details::Empty,
      Details::Exception> {
    public:
      using Type = V;
      static constexpr auto is_noexcept = Details::test_all(
        is_noexcept_reactor_v<R>...);

      /** Constructs a MultiSync.
          @param value The value to keep synchronized.
          @param reactor The reactors used to synchronize the <i>value</i>.
       */
      explicit MultiSync(Type& value, R... reactor);

      State commit(int sequence) noexcept;

      const Type& eval() const noexcept(is_noexcept);

    private:
      Type* m_value;
      StaticCommitHandler<R...> m_reactors;
  };

  template<typename V, typename... R>
  MultiSync<V, R...>::MultiSync(Type& value, R... reactor)
    : m_value(&value),
      m_reactors(std::move(reactor)...) {}

  template<typename V, typename... R>
  State MultiSync<V, R...>::commit(int sequence) noexcept {
    auto state = m_reactors.commit(sequence);
    if constexpr(!is_noexcept) {
      if(has_evaluation(state)) {
        try {
          apply([] (const auto& reactors...) {
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

  template<typename V, typename... R>
  const typename MultiSync<V, R...>::Type& MultiSync<V, R...>::eval() const
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

module;
#include <vector>

export module Aspen:VectorSync;

import :CommitHandler;
import :State;
import :Sync;
import :Traits;

export namespace Aspen {

  /** Used to keep a vector's elements synchronized with a corresponding vector
      of reactors.
      @param <V> The type of vector to synchronize.
      @param <R> The type of reactor used to synchronize the vector.
      @param <A> The type of allocator to used.
   */
  template<typename R, typename V = std::vector<reactor_result_t<R>>>
  class VectorSync : private std::conditional_t<is_noexcept_reactor_v<R>,
      Details::Empty, Details::Exception> {
    public:
      using Type = V;
      static constexpr auto is_noexcept = is_noexcept_reactor_v<R>;

      /** Constructs a VectorSync.
          @param value The vector to keep synchronized.
          @param reactors The vector of reactors used to synchronize the
                 <i>value</i>.
       */
      VectorSync(Type& value, std::vector<R> reactors);

      State commit(int sequence) noexcept;

      const Type& eval() const noexcept(is_noexcept);

    private:
      Type* m_value;
      CommitHandler<Sync<R>> m_reactors;
  };

  template<typename R, typename V>
  VectorSync<R, V>::VectorSync(Type& value, std::vector<R> reactors)
    : m_value(&value),
      m_reactors([&] {
        value.resize(reactors.size());
        auto sync_reactors = std::vector<Sync<R>>();
        for(auto i = std::size_t(0); i != reactors.size(); ++i) {
          sync_reactors.push_back(Sync(value[i], std::move(reactors[i])));
        }
        return sync_reactors;
      }()) {}

  template<typename R, typename V>
  State VectorSync<R, V>::commit(int sequence) noexcept {
    if(m_reactors.size() == 0) {
      return State::COMPLETE_EVALUATED;
    }
    auto state = m_reactors.commit(sequence);
    if constexpr(!is_noexcept) {
      if(has_evaluation(state)) {
        try {
          for(auto i = std::size_t(0); i != m_reactors.size(); ++i) {
            m_reactors.get(i).eval();
          }
          this->m_exception = nullptr;
        } catch(...) {
          this->m_exception = std::current_exception();
        }
      }
    }
    return state;
  }

  template<typename R, typename V>
  const typename VectorSync<R, V>::Type& VectorSync<R, V>::eval() const
      noexcept(is_noexcept) {
    if constexpr(!is_noexcept) {
      if(this->m_exception) {
        std::rethrow_exception(this->m_exception);
      }
    }
    return *m_value;
  }
}

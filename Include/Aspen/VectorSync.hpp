#ifndef ASPEN_VECTOR_SYNC_HPP
#define ASPEN_VECTOR_SYNC_HPP
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>
#include "Aspen/CommitHandler.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Sync.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {
namespace Details {
  struct ExceptionTally {
    std::vector<bool> m_has_exception;
    std::size_t m_count = 0;
  };
}

  /**
   * Used to keep a vector's elements synchronized with a corresponding vector
   * of reactors.
   * @param <R> The type of reactor used to synchronize the vector.
   * @param <V> The type of vector to synchronize.
   */
  template<IsReactor R, typename V = std::vector<reactor_result_t<R>>>
  class VectorSync : private std::conditional_t<is_noexcept_reactor_v<R>,
      Details::Empty, Details::ExceptionTally> {
    public:

      /** The type of vector being synchronized. */
      using Type = V;

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_reactor_v<R>;

      /**
       * Constructs a VectorSync, resizing the <i>value</i> to hold one element
       * per reactor.
       * @param value The vector to keep synchronized, which must outlive this
       *        reactor and must not be reallocated, since an element of it is
       *        kept by address.
       * @param reactors The vector of reactors used to synchronize the
       *        <i>value</i>.
       */
      VectorSync(Type& value, std::vector<R> reactors);

      State commit(std::uint64_t sequence) noexcept;
      const Type& eval() const noexcept(is_noexcept);

    private:
      using Element = Sync<R, typename Type::value_type>;
      Type* m_value;
      CommitHandler<Element> m_reactors;
  };

  template<IsReactor R, typename V>
  VectorSync<R, V>::VectorSync(Type& value, std::vector<R> reactors)
    : m_value(&value),
      m_reactors([&] {
        value.resize(reactors.size());
        auto elements = std::vector<Element>();
        elements.reserve(reactors.size());
        for(auto i = std::size_t(0); i != reactors.size(); ++i) {
          elements.push_back(Element(value[i], std::move(reactors[i])));
        }
        return elements;
      }()) {
    if constexpr(!is_noexcept) {
      this->m_has_exception.resize(m_reactors.size());
    }
  }

  template<IsReactor R, typename V>
  State VectorSync<R, V>::commit(std::uint64_t sequence) noexcept {
    if(m_reactors.size() == 0) {
      return State::COMPLETE_EVALUATED;
    }
    auto state = m_reactors.commit(sequence);
    if constexpr(!is_noexcept) {
      for(auto i : m_reactors.get_evaluated()) {
        auto has_exception =
          static_cast<bool>(m_reactors.get(i).get_exception());
        if(has_exception != this->m_has_exception[i]) {
          this->m_has_exception[i] = has_exception;
          if(has_exception) {
            ++this->m_count;
          } else {
            --this->m_count;
          }
        }
      }
    }
    return state;
  }

  template<IsReactor R, typename V>
  const typename VectorSync<R, V>::Type& VectorSync<R, V>::eval()
      const noexcept(is_noexcept) {
    if constexpr(!is_noexcept) {
      if(this->m_count != 0) {
        for(auto i = std::size_t(0); i != m_reactors.size(); ++i) {
          if(this->m_has_exception[i]) {
            m_reactors.get(i).eval();
          }
        }
      }
    }
    return *m_value;
  }
}

#endif

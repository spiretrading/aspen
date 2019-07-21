#ifndef ASPEN_BOX_HPP
#define ASPEN_BOX_HPP
#include <memory>
#include <type_traits>
#include "Aspen/LocalPtr.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {
namespace Details {
  template<typename T>
  struct box_result {
    using type = const T&;
  };

  template<>
  struct box_result<void> {
    using type = void;
  };

  template<typename T>
  using box_result_t = typename box_result<T>::type;
}

  /**
   * Wraps a reactor within a generic interface.
   * @param <T> The type that the reactor evaluates to.
   */
  template<typename T>
  class Box {
    public:
      using Type = T;

      using Result = Details::box_result_t<T>;

      /**
       * Constructs a Box.
       * @param reactor The reactor to wrap.
       */
      template<typename R>
      Box(R&& reactor);

      State commit(int sequence);

      Result eval() const;

    private:
      struct BaseWrapper {
        virtual ~BaseWrapper() = default;
        virtual State commit(int sequence) = 0;
        virtual Result eval() const = 0;
      };
      template<typename R>
      struct Wrapper final : BaseWrapper {
        try_ptr_t<R> m_reactor;

        template<typename Q>
        Wrapper(Q&& reactor);
        State commit(int sequence) override;
        Result eval() const override;
      };
      std::unique_ptr<BaseWrapper> m_reactor;
  };

  template<typename R>
  Box(R&& reactor) -> Box<reactor_result_t<R>>;

  /**
    * Boxes a reactor into a generic interface.
    * @param reactor The reactor to wrap.
    */
  template<typename R>
  auto box(R&& reactor) {
    return Box(std::forward<R>(reactor));
  }

  template<typename T>
  template<typename R>
  Box<T>::Box(R&& reactor)
    : m_reactor(std::make_unique<Wrapper<std::decay_t<R>>>(
        std::forward<R>(reactor))) {}

  template<typename T>
  State Box<T>::commit(int sequence) {
    return m_reactor->commit(sequence);
  }

  template<typename T>
  typename Box<T>::Result Box<T>::eval() const {
    return m_reactor->eval();
  }

  template<typename T>
  template<typename R>
  template<typename Q>
  Box<T>::Wrapper<R>::Wrapper(Q&& reactor)
    : m_reactor(std::forward<Q>(reactor)) {}

  template<typename T>
  template<typename R>
  State Box<T>::Wrapper<R>::commit(int sequence) {
    return m_reactor->commit(sequence);
  }

  template<typename T>
  template<typename R>
  typename Box<T>::Result Box<T>::Wrapper<R>::eval() const {
    if constexpr(std::is_same_v<Result, void>) {
      m_reactor->eval();
    } else {
      return m_reactor->eval();
    }
  }
}

#endif

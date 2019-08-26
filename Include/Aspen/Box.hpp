#ifndef ASPEN_BOX_HPP
#define ASPEN_BOX_HPP
#include <memory>
#include <type_traits>
#include "Aspen/LocalPtr.hpp"
#include "Aspen/Maybe.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * Wraps a reactor within a generic interface.
   * @param <T> The type that the reactor evaluates to.
   */
  template<typename T>
  class Box {
    public:
      using Type = T;

      using Result = eval_result_t<T>;

      /**
       * Constructs a Box.
       * @param reactor The reactor to wrap.
       */
      template<typename R, typename = std::enable_if_t<
        !std::is_base_of_v<Box, std::decay_t<R>>>>
      explicit Box(R&& reactor);

      State commit(int sequence) noexcept;

      Result eval() const;

    private:
      struct BaseWrapper {
        virtual ~BaseWrapper() = default;
        virtual State commit(int sequence) noexcept = 0;
        virtual Result eval() const = 0;
      };
      template<typename R>
      struct ByReferenceWrapper final : BaseWrapper {
        try_ptr_t<R> m_reactor;

        template<typename Q>
        ByReferenceWrapper(Q&& reactor);
        State commit(int sequence) noexcept override;
        Result eval() const override;
      };
      template<typename R>
      struct ByValueWrapper final : BaseWrapper {
        try_ptr_t<R> m_reactor;
        Maybe<Type> m_value;
        State m_state;
        int m_previous_sequence;

        template<typename Q>
        ByValueWrapper(Q&& reactor);
        State commit(int sequence) noexcept override;
        Result eval() const override;
      };
      std::shared_ptr<BaseWrapper> m_reactor;
  };

  template<typename R, typename = std::enable_if_t<
    !std::is_base_of_v<Box<reactor_result_t<to_reactor_t<R>>>,
    std::decay_t<R>>>>
  Box(R&& reactor) -> Box<reactor_result_t<to_reactor_t<R>>>;

  /**
   * Boxes a reactor into a generic interface.
   * @param reactor The reactor to wrap.
   */
  template<typename R>
  auto box(R&& reactor) {
    return Box(std::forward<R>(reactor));
  }

  template<typename T>
  template<typename R, typename>
  Box<T>::Box(R&& reactor) {
    using Reactor = to_reactor_t<R>;
    if constexpr(std::is_reference_v<decltype(
        std::declval<try_ptr_t<Reactor>>()->eval())>) {
      m_reactor = std::make_shared<ByReferenceWrapper<Reactor>>(
        std::forward<R>(reactor));
    } else {
      m_reactor = std::make_shared<ByValueWrapper<Reactor>>(
        std::forward<R>(reactor));
    }
  }

  template<typename T>
  State Box<T>::commit(int sequence) noexcept {
    return m_reactor->commit(sequence);
  }

  template<typename T>
  typename Box<T>::Result Box<T>::eval() const {
    return m_reactor->eval();
  }

  template<typename T>
  template<typename R>
  template<typename Q>
  Box<T>::ByReferenceWrapper<R>::ByReferenceWrapper(Q&& reactor)
    : m_reactor(std::forward<Q>(reactor)) {}

  template<typename T>
  template<typename R>
  State Box<T>::ByReferenceWrapper<R>::commit(int sequence) noexcept {
    return m_reactor->commit(sequence);
  }

  template<typename T>
  template<typename R>
  typename Box<T>::Result Box<T>::ByReferenceWrapper<R>::eval() const {
    if constexpr(std::is_same_v<Result, void>) {
      m_reactor->eval();
    } else {
      return m_reactor->eval();
    }
  }

  template<typename T>
  template<typename R>
  template<typename Q>
  Box<T>::ByValueWrapper<R>::ByValueWrapper(Q&& reactor)
    : m_reactor(std::forward<Q>(reactor)),
      m_state(State::NONE),
      m_previous_sequence(-1) {}

  template<typename T>
  template<typename R>
  State Box<T>::ByValueWrapper<R>::commit(int sequence) noexcept {
    if(sequence == m_previous_sequence || is_complete(m_state)) {
      return m_state;
    }
    m_state = m_reactor->commit(sequence);
    if(has_evaluation(m_state)) {
      m_value = try_call([&] { return m_reactor->eval(); });
    }
    m_previous_sequence = sequence;
    return m_state;
  }

  template<typename T>
  template<typename R>
  typename Box<T>::Result Box<T>::ByValueWrapper<R>::eval() const {
    if constexpr(std::is_same_v<Result, void>) {
      m_value.get();
    } else {
      return m_value;
    }
  }
}

#endif

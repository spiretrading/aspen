#ifndef ASPEN_BOX_HPP
#define ASPEN_BOX_HPP
#include <memory>
#include <optional>
#include <type_traits>
#include "Aspen/Maybe.hpp"
#include "Aspen/Shared.hpp"
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

      Box(const Box& box);

      Box(Box&& box) = default;

      State commit(int sequence) noexcept;

      Result eval() const;

      Box& operator =(const Box& box);

      Box& operator =(Box&& box) = default;

    private:
      struct BaseWrapper {
        virtual ~BaseWrapper() = default;
        virtual std::unique_ptr<BaseWrapper> clone() = 0;
        virtual State commit(int sequence) noexcept = 0;
        virtual Result eval() const = 0;
      };
      template<typename R>
      struct ByReferenceWrapper final : BaseWrapper {
        collapse_shared_t<R> m_reactor;

        template<typename Q, typename = std::enable_if_t<
          !std::is_base_of_v<ByReferenceWrapper, std::decay_t<Q>>>>
        ByReferenceWrapper(Q&& reactor);
        std::unique_ptr<BaseWrapper> clone() override;
        State commit(int sequence) noexcept override;
        Result eval() const override;
      };
      template<typename R>
      struct ByValueWrapper final : BaseWrapper {
        static constexpr auto is_noexcept = is_noexcept_reactor_v<R>;
        collapse_shared_t<R> m_reactor;
        std::conditional_t<is_noexcept, std::optional<Type>, Maybe<Type>>
          m_value;

        template<typename Q, typename = std::enable_if_t<
          !std::is_base_of_v<ByValueWrapper, std::decay_t<Q>>>>
        ByValueWrapper(Q&& reactor);
        std::unique_ptr<BaseWrapper> clone() override;
        State commit(int sequence) noexcept override;
        Result eval() const override;
      };
      std::unique_ptr<BaseWrapper> m_reactor;
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
    if constexpr(std::is_same_v<Type, void> || std::is_reference_v<
        decltype(std::declval<Reactor>().eval())>) {
      m_reactor = std::make_unique<ByReferenceWrapper<Reactor>>(
        std::forward<R>(reactor));
    } else {
      m_reactor = std::make_unique<ByValueWrapper<Reactor>>(
        std::forward<R>(reactor));
    }
  }

  template<typename T>
  Box<T>::Box(const Box& box)
    : m_reactor(box.m_reactor->clone()) {}

  template<typename T>
  State Box<T>::commit(int sequence) noexcept {
    return m_reactor->commit(sequence);
  }

  template<typename T>
  typename Box<T>::Result Box<T>::eval() const {
    return m_reactor->eval();
  }

  template<typename T>
  Box<T>& Box<T>::operator =(const Box& box) {
    m_reactor = box.m_reactor->clone();
    return *this;
  }

  template<typename T>
  template<typename R>
  template<typename Q, typename>
  Box<T>::ByReferenceWrapper<R>::ByReferenceWrapper(Q&& reactor)
    : m_reactor(std::forward<Q>(reactor)) {}

  template<typename T>
  template<typename R>
  std::unique_ptr<typename Box<T>::BaseWrapper>
      Box<T>::ByReferenceWrapper<R>::clone() {
    return std::make_unique<ByReferenceWrapper>(*this);
  }

  template<typename T>
  template<typename R>
  State Box<T>::ByReferenceWrapper<R>::commit(int sequence) noexcept {
    return m_reactor.commit(sequence);
  }

  template<typename T>
  template<typename R>
  typename Box<T>::Result Box<T>::ByReferenceWrapper<R>::eval() const {
    if constexpr(std::is_same_v<Result, void>) {
      m_reactor.eval();
    } else {
      return m_reactor.eval();
    }
  }

  template<typename T>
  template<typename R>
  template<typename Q, typename>
  Box<T>::ByValueWrapper<R>::ByValueWrapper(Q&& reactor)
    : m_reactor(std::forward<Q>(reactor)) {}

  template<typename T>
  template<typename R>
  std::unique_ptr<typename Box<T>::BaseWrapper>
      Box<T>::ByValueWrapper<R>::clone() {
    return std::make_unique<ByValueWrapper>(*this);
  }

  template<typename T>
  template<typename R>
  State Box<T>::ByValueWrapper<R>::commit(int sequence) noexcept {
    auto state = m_reactor.commit(sequence);
    if(has_evaluation(state)) {
      m_value = try_call(
        [&] () noexcept(is_noexcept) { return m_reactor.eval(); });
    }
    return state;
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

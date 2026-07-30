#ifndef ASPEN_BOX_HPP
#define ASPEN_BOX_HPP
#include <concepts>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include "Aspen/Maybe.hpp"
#include "Aspen/Reactor.hpp"
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

      /** The type that the reactor evaluates to. */
      using Type = T;

      /** The type returned by an evaluation. */
      using Result = eval_result_t<T>;

      /**
       * Constructs a Box.
       * @param reactor The reactor to wrap.
       */
      template<typename R> requires(
        !std::derived_from<std::remove_cvref_t<R>, Box<T>>)
      explicit Box(R&& reactor);

      State commit(std::uint64_t sequence) noexcept;
      Result eval() const;

    private:
      struct BaseWrapper {
        virtual ~BaseWrapper() = default;

        virtual State commit(std::uint64_t sequence) noexcept = 0;
        virtual Result eval() const = 0;
      };
      template<IsReactor R>
      struct ByReferenceWrapper final : BaseWrapper {
        R m_reactor;

        template<typename Q> requires std::constructible_from<R, Q>
        explicit ByReferenceWrapper(Q&& reactor);

        State commit(std::uint64_t sequence) noexcept override;
        Result eval() const override;
      };
      template<IsReactor R>
      struct ByValueWrapper final : BaseWrapper {
        static constexpr auto is_noexcept = is_noexcept_reactor_v<R>;
        R m_reactor;
        std::conditional_t<is_noexcept, std::optional<Type>, Maybe<Type>>
          m_value;

        template<typename Q> requires std::constructible_from<R, Q>
        explicit ByValueWrapper(Q&& reactor);

        State commit(std::uint64_t sequence) noexcept override;
        Result eval() const override;
      };
      std::unique_ptr<BaseWrapper> m_reactor;
  };

  template<typename R> requires(!std::derived_from<
    std::remove_cvref_t<R>, Box<reactor_result_t<to_reactor_t<R>>>>)
  Box(R&& reactor) -> Box<reactor_result_t<to_reactor_t<R>>>;

  /**
   * Boxes a reactor into a generic interface.
   * @param reactor The reactor to wrap.
   * @return A Box wrapping the <i>reactor</i>.
   */
  auto box(auto&& reactor) {
    return Box(std::forward<decltype(reactor)>(reactor));
  }

  template<typename T>
  template<typename R> requires(
    !std::derived_from<std::remove_cvref_t<R>, Box<T>>)
  Box<T>::Box(R&& reactor) {
    using Reactor = to_reactor_t<R>;
    if constexpr(std::is_same_v<Type, void> || std::is_same_v<
        decltype(std::declval<const Reactor&>().eval()), Result>) {
      m_reactor =
        std::make_unique<ByReferenceWrapper<Reactor>>(std::forward<R>(reactor));
    } else {
      m_reactor =
        std::make_unique<ByValueWrapper<Reactor>>(std::forward<R>(reactor));
    }
  }

  template<typename T>
  State Box<T>::commit(std::uint64_t sequence) noexcept {
    return m_reactor->commit(sequence);
  }

  template<typename T>
  typename Box<T>::Result Box<T>::eval() const {
    return m_reactor->eval();
  }

  template<typename T>
  template<IsReactor R>
  template<typename Q> requires std::constructible_from<R, Q>
  Box<T>::ByReferenceWrapper<R>::ByReferenceWrapper(Q&& reactor)
    : m_reactor(std::forward<Q>(reactor)) {}

  template<typename T>
  template<IsReactor R>
  State Box<T>::ByReferenceWrapper<R>::commit(std::uint64_t sequence) noexcept {
    return m_reactor.commit(sequence);
  }

  template<typename T>
  template<IsReactor R>
  typename Box<T>::Result Box<T>::ByReferenceWrapper<R>::eval() const {
    if constexpr(std::is_same_v<Result, void>) {
      m_reactor.eval();
    } else {
      return m_reactor.eval();
    }
  }

  template<typename T>
  template<IsReactor R>
  template<typename Q> requires std::constructible_from<R, Q>
  Box<T>::ByValueWrapper<R>::ByValueWrapper(Q&& reactor)
    : m_reactor(std::forward<Q>(reactor)) {}

  template<typename T>
  template<IsReactor R>
  State Box<T>::ByValueWrapper<R>::commit(std::uint64_t sequence) noexcept {
    auto state = m_reactor.commit(sequence);
    if(has_evaluation(state)) {
      m_value = try_call(
        [&] () noexcept(is_noexcept) { return m_reactor.eval(); });
    }
    return state;
  }

  template<typename T>
  template<IsReactor R>
  typename Box<T>::Result Box<T>::ByValueWrapper<R>::eval() const {
    return *m_value;
  }
}

#endif

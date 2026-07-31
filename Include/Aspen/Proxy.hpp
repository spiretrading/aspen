#ifndef ASPEN_PROXY_HPP
#define ASPEN_PROXY_HPP
#include <concepts>
#include <cstdint>
#include <optional>
#include <utility>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/Reactor.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * A reactor that forwards calls to another reactor.
   * @param <T> The type of reactor to proxy.
   */
  template<IsReactor T>
  class Proxy {
    public:

      /** The type to evaluate to. */
      using Type = reactor_result_t<T>;

      /** The type returned by an evaluation. */
      using Result = common_evaluation_t<T>;

      /** Whether this reactor's eval is noexcept. */
      static constexpr auto is_noexcept = is_noexcept_evaluation_v<T>;

      /** Constructs an empty Proxy. */
      Proxy() noexcept;

      /**
       * Sets the reactor to proxy.
       * @param reactor The reactor to forward calls to.
       */
      template<typename U> requires std::constructible_from<T, U>
      void set_reactor(U&& reactor);

      State commit(std::uint64_t sequence) noexcept;
      Result eval() const noexcept(is_noexcept);

    private:
      std::optional<T> m_reactor;
      bool m_has_cycle;
      State m_state;
      CommitFlag* m_flag;
  };

  /**
   * Makes a proxy reactor.
   * @param <T> The type of reactor to proxy.
   * @return A Proxy with no reactor to forward to.
   */
  template<IsReactor T>
  auto proxy() {
    return Proxy<T>();
  }

  template<IsReactor T>
  Proxy<T>::Proxy() noexcept
    : m_has_cycle(false),
      m_state(State::NONE),
      m_flag(nullptr) {}

  template<IsReactor T>
  template<typename U> requires std::constructible_from<T, U>
  void Proxy<T>::set_reactor(U&& reactor) {
    m_reactor.emplace(std::forward<U>(reactor));
    m_state = State::NONE;
    if(m_flag) {
      m_flag->raise();
    }
  }

  template<IsReactor T>
  State Proxy<T>::commit(std::uint64_t sequence) noexcept {
    m_flag = CommitFlag::get_current();
    if(is_complete(m_state) || m_has_cycle || !m_reactor) {
      return m_state;
    }
    m_has_cycle = true;
    auto state = m_reactor->commit(sequence);
    m_has_cycle = false;
    if(is_complete(state)) {
      m_state = state;
    } else {
      m_state = State::NONE;
    }
    return state;
  }

  template<IsReactor T>
  typename Proxy<T>::Result Proxy<T>::eval() const noexcept(is_noexcept) {
    return m_reactor->eval();
  }
}

#endif

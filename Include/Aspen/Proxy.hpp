#ifndef ASPEN_PROXY_HPP
#define ASPEN_PROXY_HPP
#include <optional>
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * A reactor that forwards calls to another reactor.
   * @param <T> The type of reactor to proxy.
   */
  template<typename T>
  class Proxy {
    public:
      using Type = reactor_result_t<T>;
      static constexpr auto is_noexcept = is_noexcept_reactor_v<T>;

      /** Constructs an empty Proxy. */
      Proxy() noexcept;

      /** Sets the reactor to proxy. */
      template<typename U>
      void set_reactor(U&& reactor);

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept(is_noexcept);

    private:
      std::optional<T> m_reactor;
      bool m_has_cycle;
      State m_state;
  };

  /**
   * Makes a proxy reactor.
   */
  template<typename T>
  auto proxy() {
    return Proxy<T>();
  }

  template<typename T>
  Proxy<T>::Proxy() noexcept
    : m_has_cycle(false),
      m_state(State::EMPTY) {}

  template<typename T>
  template<typename U>
  void Proxy<T>::set_reactor(U&& reactor) {
    m_reactor.emplace(std::forward<U>(reactor));
  }

  template<typename T>
  State Proxy<T>::commit(int sequence) noexcept {
    if(is_complete(m_state) || m_has_cycle || !m_reactor.has_value()) {
      return m_state;
    }
    m_has_cycle = true;
    auto state = m_reactor->commit(sequence);
    m_has_cycle = false;
    if(is_complete(state)) {
      m_state = state;
    } else if(is_empty(m_state) && !is_empty(state)) {
      m_state = State::NONE;
    }
    return state;
  }

  template<typename T>
  eval_result_t<typename Proxy<T>::Type> Proxy<T>::eval()
      const noexcept(is_noexcept) {
    return m_reactor->eval();
  }
}

#endif

#ifndef ASPEN_CELL_HPP
#define ASPEN_CELL_HPP
#include <cstdint>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>
#include "Aspen/CommitFlag.hpp"
#include "Aspen/State.hpp"
#include "Aspen/Traits.hpp"

namespace Aspen {

  /**
   * A reactor that evaluates to the most recently set value.
   * @param <T> The type to evaluate to.
   */
  template<typename T>
  class Cell {
    public:

      /** The type to evaluate to. */
      using Type = T;

      /** Constructs a Cell with no initial value. */
      Cell() noexcept;

      /**
       * Constructs a Cell with an initial value.
       * @param value The initial value to evaluate to.
       */
      explicit Cell(Type value) noexcept(
        std::is_nothrow_move_constructible_v<Type>);

      /**
       * Constructs a Cell by in-place constructing its initial value.
       * @param args The arguments to forward to the constructor of the value.
       */
      template<typename... A>
      explicit Cell(std::in_place_t, A&&... args) noexcept(
        std::is_nothrow_constructible_v<Type, A&&...>);

      Cell(const Cell& cell);
      Cell(Cell&& cell);

      /**
       * Sets the value to evaluate to.
       * @param value The value this reactor should evaluate to.
       */
      void set(Type value);

      /**
       * In-place constructs the value to evaluate to.
       * @param args The arguments to forward to the constructor of the value.
       */
      template<typename... A>
      void emplace(A&&... args);

      /** Brings this reactor to a completion state. */
      void set_complete();

      /**
       * Sets the value to evaluate to and brings this reactor to a completion
       * state.
       * @param value The value to evaluate to.
       */
      void set_complete(Type value);

      /**
       * In-place constructs the value to evaluate to and brings this reactor to
       * a completion state.
       * @param args The arguments to forward to the constructor of the value.
       */
      template<typename... A>
      void emplace_complete(A&&... args);

      State commit(std::uint64_t sequence) noexcept;
      eval_result_t<Type> eval() const noexcept;
      Cell& operator =(const Cell& cell);
      Cell& operator =(Cell&& cell);

    private:
      mutable std::mutex m_mutex;
      bool m_is_complete;
      std::optional<Type> m_current;
      std::optional<Type> m_next;
      CommitFlag* m_flag;

      void update(auto&& f);
  };

  template<typename T>
  Cell<T>::Cell() noexcept
    : m_is_complete(false),
      m_flag(nullptr) {}

  template<typename T>
  Cell<T>::Cell(Type value) noexcept(std::is_nothrow_move_constructible_v<Type>)
    : m_is_complete(false),
      m_next(std::move(value)),
      m_flag(nullptr) {}

  template<typename T>
  template<typename... A>
  Cell<T>::Cell(std::in_place_t, A&&... args) noexcept(
    std::is_nothrow_constructible_v<Type, A&&...>)
    : m_is_complete(false),
      m_next(std::in_place, std::forward<A>(args)...),
      m_flag(nullptr) {}

  template<typename T>
  Cell<T>::Cell(const Cell& cell)
      : m_flag(nullptr) {
    auto lock = std::lock_guard(cell.m_mutex);
    m_is_complete = cell.m_is_complete;
    if(cell.m_next) {
      m_next = cell.m_next;
    } else {
      m_next = cell.m_current;
    }
  }

  template<typename T>
  Cell<T>::Cell(Cell&& cell)
      : m_flag(nullptr) {
    auto lock = std::lock_guard(cell.m_mutex);
    m_is_complete = cell.m_is_complete;
    if(cell.m_next) {
      m_next = std::move(cell.m_next);
    } else {
      m_next = std::move(cell.m_current);
    }
  }

  template<typename T>
  void Cell<T>::set(Type value) {
    update([&] {
      m_next = std::move(value);
    });
  }

  template<typename T>
  template<typename... A>
  void Cell<T>::emplace(A&&... args) {
    update([&] {
      m_next.emplace(std::forward<A>(args)...);
    });
  }

  template<typename T>
  void Cell<T>::set_complete() {
    update([&] {
      m_is_complete = true;
    });
  }

  template<typename T>
  void Cell<T>::set_complete(Type value) {
    update([&] {
      m_next = std::move(value);
      m_is_complete = true;
    });
  }

  template<typename T>
  template<typename... A>
  void Cell<T>::emplace_complete(A&&... args) {
    update([&] {
      m_next.emplace(std::forward<A>(args)...);
      m_is_complete = true;
    });
  }

  template<typename T>
  State Cell<T>::commit(std::uint64_t sequence) noexcept {
    auto lock = std::lock_guard(m_mutex);
    m_flag = CommitFlag::get_current();
    auto state = State::NONE;
    if(m_next) {
      m_current = std::move(m_next);
      m_next = std::nullopt;
      state = State::EVALUATED;
    }
    if(m_is_complete) {
      state = combine(state, State::COMPLETE);
    }
    return state;
  }

  template<typename T>
  eval_result_t<typename Cell<T>::Type> Cell<T>::eval() const noexcept {
    return *m_current;
  }

  template<typename T>
  Cell<T>& Cell<T>::operator =(const Cell& cell) {
    if(this == &cell) {
      return *this;
    }
    auto flag = [&] {
      auto lock = std::scoped_lock(m_mutex, cell.m_mutex);
      m_is_complete = cell.m_is_complete;
      if(cell.m_next) {
        m_next = cell.m_next;
      } else {
        m_next = cell.m_current;
      }
      return m_flag;
    }();
    if(flag) {
      flag->raise();
    }
    return *this;
  }

  template<typename T>
  Cell<T>& Cell<T>::operator =(Cell&& cell) {
    if(this == &cell) {
      return *this;
    }
    auto flag = [&] {
      auto lock = std::scoped_lock(m_mutex, cell.m_mutex);
      m_is_complete = cell.m_is_complete;
      if(cell.m_next) {
        m_next = std::move(cell.m_next);
      } else {
        m_next = std::move(cell.m_current);
      }
      return m_flag;
    }();
    if(flag) {
      flag->raise();
    }
    return *this;
  }

  template<typename T>
  void Cell<T>::update(auto&& f) {
    auto flag = [&] () -> CommitFlag* {
      auto lock = std::lock_guard(m_mutex);
      if(m_is_complete) {
        return nullptr;
      }
      f();
      return m_flag;
    }();
    if(flag) {
      flag->raise();
    }
  }
}

#endif

export module Aspen:Cell;

import std;
import :State;
import :Traits;
import :Trigger;

export namespace Aspen {

  /**
   * A reactor that evaluates to the most recently set value.
   * @param <T> The type to evaluate to.
   */
  template<typename T>
  class Cell {
    public:
      using Type = T;

      /** Constructs an Cell with a default initial value. */
      Cell();

      /**
       * Constructs a Cell with an initial value.
       * @param value The initial value to evaluate to.
       */
      explicit Cell(Type value);

      /**
       * Constructs a Cell by in-place constructing its initial value.
       * @param args The arguments to forward to the constructor of the value.
       */
      template<typename... A>
      explicit Cell(std::in_place_t, A&&... args);

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

      State commit(int sequence) noexcept;

      eval_result_t<Type> eval() const noexcept;

      Cell& operator =(const Cell& cell);

      Cell& operator =(Cell&& cell);

    private:
      std::mutex m_mutex;
      bool m_is_complete;
      std::optional<Type> m_current;
      std::optional<Type> m_next;
      Trigger* m_trigger;
  };

  template<typename T>
  Cell<T>::Cell()
    : m_is_complete(false),
      m_trigger(nullptr) {}

  template<typename T>
  Cell<T>::Cell(Type value)
    : m_is_complete(false),
      m_next(std::move(value)),
      m_trigger(nullptr) {}

  template<typename T>
  template<typename... A>
  Cell<T>::Cell(std::in_place_t, A&&... args)
    : m_is_complete(false),
      m_next(std::in_place, std::forward<A>(args)...),
      m_trigger(nullptr) {}

  template<typename T>
  Cell<T>::Cell(const Cell& cell)
      : m_trigger(nullptr) {
    auto lock = std::lock_guard(cell.m_mutex);
    m_is_complete = cell.m_is_complete;
    m_current = cell.m_current;
    m_next = cell.m_next;
  }

  template<typename T>
  Cell<T>::Cell(Cell&& cell)
      : m_trigger(nullptr) {
    auto lock = std::lock_guard(cell.m_mutex);
    m_is_complete = cell.m_is_complete;
    m_current = std::move(cell.m_current);
    m_next = std::move(cell.m_next);
  }

  template<typename T>
  void Cell<T>::set(Type value) {
    auto lock = std::lock_guard(m_mutex);
    m_next = std::move(value);
    if(m_trigger != nullptr) {
      m_trigger->signal();
    }
  }

  template<typename T>
  template<typename... A>
  void Cell<T>::emplace(A&&... args) {
    auto lock = std::lock_guard(m_mutex);
    m_next.emplace(std::forward<A>(args)...);
    if(m_trigger != nullptr) {
      m_trigger->signal();
    }
  }

  template<typename T>
  void Cell<T>::set_complete() {
    auto lock = std::lock_guard(m_mutex);
    m_is_complete = true;
    if(m_trigger != nullptr) {
      m_trigger->signal();
    }
  }

  template<typename T>
  void Cell<T>::set_complete(Type value) {
    auto lock = std::lock_guard(m_mutex);
    m_is_complete = true;
    m_next = std::move(value);
    if(m_trigger != nullptr) {
      m_trigger->signal();
    }
  }

  template<typename T>
  template<typename... A>
  void Cell<T>::emplace_complete(A&&... args) {
    auto lock = std::lock_guard(m_mutex);
    m_is_complete = true;
    m_next.emplace(std::forward<A>(args)...);
    if(m_trigger != nullptr) {
      m_trigger->signal();
    }
  }

  template<typename T>
  State Cell<T>::commit(int sequence) noexcept {
    auto lock = std::lock_guard(m_mutex);
    if(m_trigger == nullptr) {
      m_trigger = Trigger::get_trigger();
    }
    auto state = State::NONE;
    if(m_next.has_value()) {
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
    auto lock = std::lock_guard(m_mutex);
    auto cell_lock = std::lock_guard(cell.m_mutex);
    m_is_complete = cell.m_is_complete;
    m_current = cell.m_current;
    m_next = cell.m_next;
    return *this;
  }

  template<typename T>
  Cell<T>& Cell<T>::operator =(Cell&& cell) {
    auto lock = std::lock_guard(m_mutex);
    auto cell_lock = std::lock_guard(cell.m_mutex);
    m_is_complete = std::move(cell.m_is_complete);
    m_current = std::move(cell.m_current);
    m_next = std::move(cell.m_next);
    return *this;
  }
}

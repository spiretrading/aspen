#ifndef ASPEN_QUEUE_HPP
#define ASPEN_QUEUE_HPP
#include <exception>
#include "Aspen/State.hpp"

namespace Aspen {

  /**
   * A reactor that evaluates to the values pushed to an internal queue.
   * @param <T> The type of values to queue.
   */
  template<typename T>
  class Queue {
    public:
      using Type = T;

      /** Constructs an empty Queue. */
      Queue();

      /**
       * Pushes a value to the queue.
       * @param value The value to push.
       */
      void push(Type value);

      /** Brings this reactor to a completion state. */
      void set_complete();

      /**
       * Brings this reactor to a completion state by throwing an exception.
       * @param e The exception to throw.
       */
      void set_complete(std::exception_ptr e);

      /**
       * Brings this reactor to a completion state by throwing an exception.
       * @param e The exception to throw.
       */
      template<typename E>
      void set_complete(const E& e);

      State commit(int sequence);

      const Type& eval() const;
  };

  template<typename T>
  Queue<T>::Queue() {}

  template<typename T>
  void Queue<T>::push(Type value) {}

  template<typename T>
  void Queue<T>::set_complete() {}

  template<typename T>
  void Queue<T>::set_complete(std::exception_ptr e) {}

  template<typename T>
  template<typename E>
  void Queue<T>::set_complete(const E& e) {
    set_complete(std::make_exception_ptr(e));
  }

  template<typename T>
  State Queue<T>::commit(int sequence) {
    return {};
  }

  template<typename T>
  const typename Queue<T>::Type& Queue<T>::eval() const {
    return {};
  }
}

#endif

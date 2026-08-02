#ifndef ASPEN_CONCURRENCY_TESTS_HPP
#define ASPEN_CONCURRENCY_TESTS_HPP
#include <cstdlib>

namespace Aspen::Tests {

  /** Returns the number of iterations each concurrency test performs. */
  inline int get_iterations() {
    auto variable = std::getenv("ASPEN_CONCURRENCY_ITERATIONS");
    if(!variable) {
      return 1000;
    }
    auto count = std::atoi(variable);
    if(count <= 0) {
      return 1000;
    }
    return count;
  }
}

#endif

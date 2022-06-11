#include "combinatorics/partition.h"

namespace chalk {

Integer Factorial(uint64_t n) {
  Integer result = 1;
  for (uint64_t i = 1; i <= n; ++i) { result *= i; }
  return result;
}

}  // namespace chalk

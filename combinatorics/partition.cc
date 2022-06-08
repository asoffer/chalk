#include "combinatorics/partition.h"

namespace chalk {

uint64_t Factorial(uint64_t n) {
  assert(n <= 20 && "Support for partitions of no more than 20");
  uint64_t result = 1;
  for (uint64_t i = 1; i <= n; ++i) { result *= i; }
  return result;
}

}  // namespace chalk

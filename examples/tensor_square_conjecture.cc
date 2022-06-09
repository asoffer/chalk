#include <iostream>
#include <vector>

#include "absl/types/span.h"
#include "combinatorics/partition.h"
#include "symmetric_group/character.h"

size_t ValidateInputs(absl::Span<char const * const> input) {
  if (input.size() != 2) {
    std::cerr << "Must have exactly one input.\n";
    std::abort();
  }

  size_t n = std::atoi(input[1]);
  if (n == 0) {
    std::cerr << "Input must be a positive integer.\n";
    std::abort();
  }

  return n;
}

chalk::Partition StaircasePartition(size_t n) {
  std::vector<uint8_t> v;
  for (size_t i = 1; i <= n; ++i) { v.push_back(i); }
  return chalk::Partition(v.begin(), v.end());
}

int main(int argc, char const *argv[]) {
  size_t n = ValidateInputs(absl::MakeConstSpan(argv, argc));
  auto staircase =
      chalk::SymmetricGroupCharacter::Irreducible(StaircasePartition(n));
  auto tensor_square = staircase * staircase;
  std::cerr << tensor_square;

  for (auto p : chalk::Partition::All(n * (n + 1) / 2)) {
    std::cerr << "Inner product with " << p << ": "
              << InnerProduct(tensor_square,
                              chalk::SymmetricGroupCharacter::Irreducible(p))
              << "\n";
  }
  return 0;
}

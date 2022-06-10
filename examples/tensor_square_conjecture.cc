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
  size_t N = n * (n + 1) / 2;

  auto staircase =
      chalk::SymmetricGroupCharacter::Irreducible(StaircasePartition(n));
  auto tensor_square = staircase * staircase;
  std::cerr << tensor_square;

  std::vector irreducibles = chalk::SymmetricGroupCharacter::AllIrreducibles(N);
  for (size_t i = 0; i < irreducibles.size(); ++i) {
    if (InnerProduct(tensor_square, irreducibles[i]) != 0) { continue; }

    auto range = chalk::Partition::All(N);
    auto iter  = range.begin();
    for (size_t j = 0; j < i; ++j) { ++iter; }
    std::cerr << "Tensor square for the staircase shape of size " << n
              << " does not contain any copies of the irreducible "
                 "representation for "
              << *iter << ".\n";
    return 0;
  }
  std::cerr << "Conjecture validated for " << n << ".\n";
  return 0;
}

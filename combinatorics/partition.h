#ifndef CHALK_COMBINATORICS_PARTITION_H
#define CHALK_COMBINATORICS_PARTITION_H

#include <algorithm>
#include <cassert>
#include <concepts>
#include <ostream>
#include <string>
#include <string_view>

namespace chalk {

// Represents an integer partition, where each part is stored as a `PartType` in
// contiguous storage in weakly descending order.
template <std::integral PartType>
struct BasicPartition {
  using value_type     = PartType;
  using const_iterator = typename std::basic_string<value_type>::const_iterator;

  // Constructs a partition from the given parts.
  BasicPartition(std::initializer_list<value_type> parts)
      : parts_(parts.begin(), parts.end()) {
    std::sort(parts_.rbegin(), parts_.rend());
  }

  static BasicPartition Trivial() {
    return BasicPartition(std::basic_string<value_type>());
  }
  static BasicPartition Rectangle(size_t count, value_type n) {
    return BasicPartition(std::basic_string<value_type>(count, n));
  }
  static BasicPartition MaximallyDivided(value_type n) {
    return Rectangle(n, 1);
  }
  static BasicPartition Full(value_type n) { return Rectangle(1, n); }


  // Returns the number of parts in the partition.
  constexpr size_t parts() { return parts_.size(); }

  // Returns the integer `n` for which `*this` is a partition.
  constexpr size_t whole() const {
    size_t total = 0;
    for (size_t n : parts_) { total += n; }
    return total;
  }

  // Returns the conjugate partition.
  BasicPartition conjugate() const {
    BasicPartition result((std::basic_string<value_type>()));
    result.parts_.reserve(parts_[0]);
    value_type last_part_size = 0;
    auto const next_largest = [&](value_type n) { return n > last_part_size; };

    for (auto iter = parts_.crbegin(); iter != parts_.crend();
         iter      = std::find_if(iter, parts_.crend(), next_largest)) {
      for (; last_part_size < *iter; ++last_part_size) {
        result.parts_.push_back(std::distance(iter, parts_.crend()));
      }
    }
    return result;
  }

  auto begin() const { return parts_.begin(); }
  auto cbegin() const { return parts_.cbegin(); }
  auto rbegin() const { return parts_.rbegin(); }
  auto crbegin() const { return parts_.crbegin(); }
  auto end() { return parts_.end(); }
  auto end() const { return parts_.end(); }
  auto cend() const { return parts_.cend(); }
  auto crend() const { return parts_.crend(); }

  value_type operator[](size_t i) const {
    assert(i < parts_.size());
    return parts_[i];
  }

  friend bool operator==(BasicPartition const &lhs, BasicPartition const &rhs) {
    return lhs.parts_ == rhs.parts_;
  }

  friend bool operator!=(BasicPartition const &lhs, BasicPartition const &rhs) {
    return not(lhs == rhs);
  }

  friend std::ostream &operator<<(std::ostream &os, BasicPartition const &p) {
    std::string_view separator = "(";
    for (auto part : p.parts_) {
      os << std::exchange(separator, ", ") << uint64_t{part};
    }
    return os << ")";
  }

 private:
  BasicPartition(std::basic_string<value_type> parts)
      : parts_(std::move(parts)) {}

  std::basic_string<value_type> parts_;
};

using Partition = BasicPartition<uint8_t>;

// Computes the product of all positive integers less than or equal to `n`.
uint64_t Factorial(uint64_t n);

// Computes the factorial of the partition `p`. That is, the product of the
// factorial of all of the parts in the partition.
template <std::integral PartType>
uint64_t Factorial(BasicPartition<PartType> const &p) {
  assert(p.whole() <= 20 && "Support for partitions of no more than 20");
  uint64_t result = 1;
  for (uint64_t n : p) { result *= Factorial(n); }
  return result;
}

// Returns the number of permutations in the symmetric group with cycle-type
// `p`.
template <std::integral PartType>
uint64_t CycleTypeCount(BasicPartition<PartType> const &p) {
  uint64_t denominator    = 1;
  PartType last_part_size = 0;
  auto const next_largest = [&](PartType n) { return n != last_part_size; };
  auto iter               = p.cbegin();
  while (iter != p.cend()) {
    auto next_iter = std::find_if(iter, p.cend(), next_largest);
    denominator *= Factorial(std::distance(iter, next_iter));
    iter           = next_iter;
    last_part_size = *iter;
  }

  uint64_t numerator = Factorial(p.whole());
  for (uint64_t n : p) { numerator /= n; }

  return numerator / denominator;
}

// Returns the rank of the partition `p`. That is, returns the largest value `n`
// for which the partition contains `n` parts of size at least `n`.
template <std::integral PartType>
PartType Rank(BasicPartition<PartType> const &p) {
  PartType n = 0;
  for (auto iter = p.begin(); iter != p.end(); ++iter) {
    if (*iter <= n) return n;
    ++n;
  }
  return n;
}

}  // namespace chalk

#endif  // CHALK_COMBINATORICS_PARTITION_H

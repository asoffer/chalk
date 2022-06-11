#ifndef CHALK_COMBINATORICS_PARTITION_H
#define CHALK_COMBINATORICS_PARTITION_H

#include <algorithm>
#include <cassert>
#include <concepts>
#include <ostream>
#include <string>
#include <string_view>

#include "base/iterator.h"
#include "integer.h"

namespace chalk {

template <std::integral PartType>
struct BasicPartition;

namespace internal_partition {

template <std::integral PartType>
struct AllPartitionsIterator {
  using value_type = BasicPartition<PartType>;

  explicit AllPartitionsIterator(BasicPartition<PartType> p)
      : p_(std::move(p)) {}

  bool operator==(AllPartitionsIterator const &) const = default;
  bool operator!=(AllPartitionsIterator const &) const = default;

  value_type const &operator*() const { return p_; }
  value_type const *operator->() const { return &p_; }

  AllPartitionsIterator &operator++() {
    auto iter = std::partition_point(p_.parts_.begin(), p_.parts_.end(),
                                     [](PartType n) { return n != 1; });
    if (iter == p_.parts_.begin()) {
      p_ = {};
      return *this;
    }
    // Subtract 1 from the entry immediately before `iter`.
    --*std::prev(iter);
    size_t value = *std::prev(iter);

    // Count the number of ones including the one we just extracted.
    size_t num_ones = std::distance(iter, p_.parts_.end()) + 1;
    p_.parts_.erase(iter, p_.parts_.end());
    for (; num_ones > value; num_ones -= value) { p_.parts_.push_back(value); }
    p_.parts_.push_back(num_ones);
    return *this;
  }

  AllPartitionsIterator operator++(int) {
    auto copy = *this;
    ++*this;
    return copy;
  }

 private:
  BasicPartition<PartType> p_;
};

}  // namespace internal_partition

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

  template <typename Iter>
  explicit BasicPartition(Iter b, Iter e) : parts_(b, e) {
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
  constexpr size_t parts() const { return parts_.size(); }

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
  auto end() const { return parts_.end(); }
  auto cend() const { return parts_.cend(); }
  auto rend() const { return parts_.rend(); }
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

  template <typename H>
  friend H AbslHashValue(H h, BasicPartition const &p) {
    return H::combine_contiguous(std::move(h), &*p.parts_.begin(), p.parts());
  }

  friend std::ostream &operator<<(std::ostream &os, BasicPartition const &p) {
    std::string_view separator = "(";
    for (auto part : p.parts_) {
      os << std::exchange(separator, ", ") << uint64_t{part};
    }
    return os << ")";
  }

  // Returns an iterator range that will iterate through all partitions of size
  // `n` in lexicographic order.
  static auto All(value_type n) {
    assert(n > 0);
    return base::iterator_range(
        internal_partition::AllPartitionsIterator<value_type>(Full(n)),
        internal_partition::AllPartitionsIterator<value_type>(
            BasicPartition{}));
  }

 private:
  friend struct internal_partition::AllPartitionsIterator<value_type>;
  explicit BasicPartition(std::basic_string<value_type> parts)
      : parts_(std::move(parts)) {}

  std::basic_string<value_type> parts_;
};

using Partition = BasicPartition<uint8_t>;

// Computes the product of all positive integers less than or equal to `n`.
Integer Factorial(uint64_t n);

// Computes the factorial of the partition `p`. That is, the product of the
// factorial of all of the parts in the partition.
template <std::integral PartType>
Integer Factorial(BasicPartition<PartType> const &p) {
  Integer result = 1;
  for (uint64_t n : p) { result *= Factorial(n); }
  return result;
}

// Returns the number of permutations in the symmetric group with cycle-type
// `p`.
template <std::integral PartType>
Integer CycleTypeCount(BasicPartition<PartType> const &p) {
  Integer denominator     = 1;
  PartType last_part_size = 0;
  auto const next_largest = [&](PartType n) { return n != last_part_size; };
  auto iter               = p.cbegin();
  while (iter != p.cend()) {
    auto next_iter = std::find_if(iter, p.cend(), next_largest);
    denominator *= Factorial(std::distance(iter, next_iter));
    iter           = next_iter;
    last_part_size = *iter;
  }

  Integer numerator = Factorial(p.whole());
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

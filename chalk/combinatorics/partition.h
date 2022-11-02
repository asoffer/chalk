#ifndef CHALK_COMBINATORICS_PARTITION_H
#define CHALK_COMBINATORICS_PARTITION_H

#include <algorithm>
#include <cassert>
#include <concepts>
#include <iostream>
#include <string>
#include <string_view>

#include "chalk/base/iterator.h"
#include "chalk/combinatorics/composition.h"
#include "chalk/integer.h"

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
struct BasicPartition : private BasicComposition<PartType> {
 private:
  using Base = BasicComposition<PartType>;

 public:
  using composition_type = Base;
  using value_type       = PartType;
  using const_iterator   = typename Base::const_iterator;

  // Constructs the unique partition of 0.
  BasicPartition() = default;

  // Constructs a partition from the given parts.
  BasicPartition(std::initializer_list<value_type> parts)
      : Base(parts.begin(), parts.end()) {
    std::sort(Base::rbegin(), Base::rend());
  }

  template <typename Iter>
  explicit BasicPartition(Iter b, Iter e) : Base(b, e) {
    std::sort(Base::rbegin(), Base::rend());
  }

  static BasicPartition Trivial() { return BasicPartition(); }
  static BasicPartition Rectangle(size_t count, value_type n) {
    return BasicPartition(std::basic_string<value_type>(count, n));
  }
  static BasicPartition MaximallyDivided(value_type n) {
    return Rectangle(n, 1);
  }
  static BasicPartition Full(value_type n) { return Rectangle(1, n); }

  // Returns a pair consisting of the partition whose parts are identical to the
  // parts in `composition` (with multiplicity) and a bool indicating whether
  // `composition` was already sorted in decreasing order (i.e., the composition
  // was already a valid partition
  static std::pair<BasicPartition, bool> From(composition_type composition) {
    BasicPartition p(std::move(composition).extract());
    bool sorted = std::is_sorted(p.rbegin(), p.rend());
    if (not sorted) { std::sort(p.Base::rbegin(), p.Base::rend()); }
    return std::pair<BasicPartition, bool>(std::move(p), sorted);
  }

  // Returns the number of parts in the partition.
  using Base::parts;

  // Returns the integer `n` for which `*this` is a partition.
  using Base::whole;

  // Returns the conjugate partition.
  BasicPartition conjugate() const {
    BasicPartition result;
    if (this->parts_.empty()) { return result; }
    result.parts_.reserve(operator[](0));
    value_type last_part_size = 0;
    auto const next_largest = [&](value_type n) { return n > last_part_size; };

    for (auto iter = crbegin(); iter != crend();
         iter      = std::find_if(iter, crend(), next_largest)) {
      for (; last_part_size < *iter; ++last_part_size) {
        result.append(std::distance(iter, crend()));
      }
    }
    return result;
  }

  value_type const &operator[](size_t n) const { return Base::operator[](n); }

  auto begin() const { return Base::begin(); }
  auto rbegin() const { return Base::rbegin(); }
  auto cbegin() const { return Base::cbegin(); }
  auto crbegin() const { return Base::crbegin(); }
  auto end() const { return Base::end(); }
  auto rend() const { return Base::rend(); }
  auto cend() const { return Base::cend(); }
  auto crend() const { return Base::crend(); }

  friend bool operator==(BasicPartition const &lhs, BasicPartition const &rhs) {
    return static_cast<Base const &>(lhs) == static_cast<Base const &>(rhs);
  }

  friend bool operator!=(BasicPartition const &lhs, BasicPartition const &rhs) {
    return not(lhs == rhs);
  }

  template <typename H>
  friend H AbslHashValue(H h, BasicPartition const &p) {
    return H::combine_contiguous(std::move(h), &*p.parts_.begin(), p.parts());
  }

  friend std::ostream &operator<<(std::ostream &os, BasicPartition const &p) {
    return os << static_cast<Base const &>(p);
  }

  // Returns an iterator range that will iterate through all partitions of
  // size `n` in lexicographic order.
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
      : Base(std::move(parts)) {}
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

// Returns the rank of the partition `p`. That is, returns the largest value
// `n` for which the partition contains `n` parts of size at least `n`.
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

#ifndef CHALK_SYMMETRIC_GROUP_CHARACTER_H
#define CHALK_SYMMETRIC_GROUP_CHARACTER_H

#include <cstdint>

#include "absl/container/flat_hash_map.h"
#include "combinatorics/partition.h"

namespace chalk {

// Represents a character in a symmetric group.
struct SymmetricGroupCharacter {
  // Constructs the character corresponding to the irreducible representation of
  // shape `p`.
  static SymmetricGroupCharacter Irreducible(Partition const &p);

  // Returns a container consisting of all irreducible characters for the
  // symmetric group on `n` elements.
  static std::vector<SymmetricGroupCharacter> AllIrreducibles(size_t n);

  // Constructs the character which evaluates to 1 on `p` and 0 elsewhere.
  static SymmetricGroupCharacter KroneckerDelta(Partition const &p);

  friend bool operator==(SymmetricGroupCharacter const &lhs,
                         SymmetricGroupCharacter const &rhs) {
    if (lhs.values_.size() != rhs.values_.size()) { return false; }
    for (auto const &[partition, coefficient] : lhs.values_) {
      auto iter = rhs.values_.find(partition);
      if (iter == rhs.values_.end()) { return false; }
      if (iter->second != coefficient) { return false; }
    }
    return true;
  }

  friend bool operator!=(SymmetricGroupCharacter const &lhs,
                         SymmetricGroupCharacter const &rhs) {
    return not(lhs == rhs);
  }

  SymmetricGroupCharacter &operator+=(SymmetricGroupCharacter const &rhs);
  SymmetricGroupCharacter &operator-=(SymmetricGroupCharacter const &rhs);

  friend SymmetricGroupCharacter operator+(SymmetricGroupCharacter lhs,
                                           SymmetricGroupCharacter const &rhs) {
    return lhs += rhs;
  }

  friend SymmetricGroupCharacter operator-(SymmetricGroupCharacter lhs,
                                           SymmetricGroupCharacter const &rhs) {
    return lhs -= rhs;
  }

  SymmetricGroupCharacter operator-() const;

  SymmetricGroupCharacter &operator*=(int64_t n);

  friend SymmetricGroupCharacter operator*(int64_t n,
                                           SymmetricGroupCharacter c) {
    return c *= n;
  }

  friend SymmetricGroupCharacter operator*(SymmetricGroupCharacter c,
                                           int64_t n) {
    return c *= n;
  }

  friend SymmetricGroupCharacter operator*(SymmetricGroupCharacter const &lhs,
                                           SymmetricGroupCharacter const &rhs) {
    SymmetricGroupCharacter result;
    for (auto const &[partition, coefficient] : lhs.values_) {
      auto iter = rhs.values_.find(partition);
      if (iter == rhs.values_.end()) { continue; }
      result.values_.try_emplace(partition, coefficient * iter->second);
    }
    return result;
  }

  SymmetricGroupCharacter &operator*=(SymmetricGroupCharacter const &rhs) {
    *this = *this * rhs;
    return *this;
  }

  friend double InnerProduct(SymmetricGroupCharacter const &lhs,
                             SymmetricGroupCharacter const &rhs) {
    if (lhs.values_.empty()) { return 0; }
    int64_t result = 0;
    for (auto const &[partition, coefficient] : lhs.values_) {
      auto iter = rhs.values_.find(partition);
      if (iter == rhs.values_.end()) { continue; }
      result += (iter->second * coefficient * CycleTypeCount(partition));
    }
    return static_cast<double>(result) /
           Factorial(lhs.values_.begin()->first.whole());
  }

  friend std::ostream &operator<<(std::ostream &os,
                                  SymmetricGroupCharacter const &c) {
    for (auto const &[partition, coefficient] : c.values_) {
      os << partition << " |-> " << coefficient << "\n";
    }
    return os;
  }

 private:
  absl::flat_hash_map<Partition, int64_t> values_;
};

}  // namespace chalk

#endif  // CHALK_SYMMETRIC_GROUP_CHARACTER_H

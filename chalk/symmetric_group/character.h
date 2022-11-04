#ifndef CHALK_SYMMETRIC_GROUP_CHARACTER_H
#define CHALK_SYMMETRIC_GROUP_CHARACTER_H

#include <cstdint>

#include "absl/container/flat_hash_map.h"
#include "chalk/algebra/property.h"
#include "chalk/combinatorics/partition.h"

namespace chalk {

// Represents a character in a symmetric group.
struct SymmetricGroupCharacter : Algebraic {
  using chalk_properties = void(Ring, Commutative<'*'>);

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

  SymmetricGroupCharacter &operator+=(SymmetricGroupCharacter const &rhs);
  SymmetricGroupCharacter &operator-=(SymmetricGroupCharacter const &rhs);

  friend SymmetricGroupCharacter operator-(SymmetricGroupCharacter lhs,
                                           SymmetricGroupCharacter const &rhs) {
    return lhs -= rhs;
  }

  SymmetricGroupCharacter operator-() const;

  SymmetricGroupCharacter &operator*=(int64_t n);

  SymmetricGroupCharacter &operator*=(SymmetricGroupCharacter const &rhs) {
    SymmetricGroupCharacter result;
    for (auto const &[partition, coefficient] : values_) {
      auto iter = rhs.values_.find(partition);
      if (iter == rhs.values_.end()) { continue; }
      result.values_.try_emplace(partition, coefficient * iter->second);
    }
    return *this = std::move(result);
  }

  friend double InnerProduct(SymmetricGroupCharacter const &lhs,
                             SymmetricGroupCharacter const &rhs) {
    if (lhs.values_.empty()) { return 0; }
    Integer result = 0;
    for (auto const &[partition, coefficient] : lhs.values_) {
      auto iter = rhs.values_.find(partition);
      if (iter == rhs.values_.end()) { continue; }
      result += (iter->second * coefficient * CycleTypeCount(partition));
    }
    // TODO: Have this return a Rational once that exists.
    return static_cast<double>(static_cast<int64_t>(result)) /
           static_cast<int64_t>(Factorial(lhs.values_.begin()->first.whole()));
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

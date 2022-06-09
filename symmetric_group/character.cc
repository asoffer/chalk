#include "symmetric_group/character.h"

#include <cassert>

namespace chalk {
namespace {

// `BoundaryPartition` represents an integer partition by representing the
// sequence of
//
// For example, given the partition (5,3,3,1), the Ferrers diagram with its
// boundary drawn in would be:
//
//   . . . . .|
//   . . .+---+
//   . . .|
//   .+---+
//   -+
//
// The boundary can be expressed as a sequence of horizontal and vertical
// movements from bottom-left to top-right. In this case, the sequence would be
// HVHHVVHHV (with 'H' for horizontal and 'V' for vertical). `BoundaryPartition`
// stores the partition in this format as a sequence of bits (0 for horizontal
// motion, 1 for vertical motion) in this bottom-left to top-right direction.
//
// This representation makes it particularly efficient to find `k`-hooks. Notice
// that a 1-hook is simply an H followed by a V. Here are all of the possible
// patterns representing hooks of small length.
//
// * Length 1 hooks: HV
// * Length 2 hooks: HHV, HVV
// * Length 3 hooks: HHHV, HHVV, HVHV, HVVV
//
// In particular, a hook of length `k` can be identified by finding a sequence
// of length `k + 1` beginning with an `H` and ending with a `V`. Moreover,
// removing the hook can be done by replacing the leading `H` with a `V` and the
// trailing `V` with an `H`.
//
struct BoundaryPartition {
  template <std::integral T>
  BoundaryPartition(BasicPartition<T> const &p) {
    typename BasicPartition<T>::value_type previous_part = 0;
    size_t index                                         = 0;

    for (auto iter = p.rbegin(); iter != p.rend(); ++iter) {
      if (previous_part == *iter) {
        shape_.set(index);
      } else {
        index += (*iter - previous_part);
        previous_part = *iter;
        shape_.set(index);
      }
      ++index;
    }
  }

  // Returns a `uint64_t` where the `n`th bit is set if and only if there is a hook of length `k`
  uint64_t hook_indices(size_t k) const {
    return (~shape_ & (shape_ >> k)).to_ullong();
  }

  // Removes the hook from the partition, returning its height (i.e., the number
  // of rows it touches)
  size_t remove_hook(size_t hook_length, size_t index) {
    assert(not shape_[index]);
    assert(shape_[index + hook_length]);
    size_t height = 1;
    for (size_t i = index + 1; i < index + hook_length; ++i) {
      height += shape_[i];
    }
    shape_.flip(index);
    shape_.flip(index + hook_length);
    return height;
  }

 private:
  std::bitset<64> shape_;
};

constexpr uint64_t BitIndex(uint64_t n) {
  uint64_t result = 0;
  if (n & 0b1111111111111111111111111111111100000000000000000000000000000000) {
    result += 32;
  }
  if (n & 0b1111111111111111000000000000000011111111111111110000000000000000) {
    result += 16;
  }
  if (n & 0b1111111100000000111111110000000011111111000000001111111100000000) {
    result += 8;
  }
  if (n & 0b1111000011110000111100001111000011110000111100001111000011110000) {
    result += 4;
  }
  if (n & 0b1100110011001100110011001100110011001100110011001100110011001100) {
    result += 2;
  }
  if (n & 0b1010101010101010101010101010101010101010101010101010101010101010) {
    result += 1;
  }
  return result;
}

constexpr uint64_t RemoveLeastSignificantBit(uint64_t &n) {
  uint64_t one_less = n - uint64_t{1};
  uint64_t index    = BitIndex(((n ^ one_less) + 1) >> 1);
  n &= one_less;
  return index;
}

// Uses the Murnaghan-Nakayama rule to compute the character value at `q` for an
// irreducible representation of shape `p`.
int64_t MurnaghanNakayama(BoundaryPartition const &p,
                          absl::Span<Partition::value_type const> parts) {
  if (parts.empty()) { return 1; }

  int64_t result = 0;

  auto hook_length = parts.front();
  parts.remove_prefix(1);

  uint64_t indices = p.hook_indices(hook_length);
  while (indices != 0) {
    BoundaryPartition q = p;
    size_t lsb_index    = RemoveLeastSignificantBit(indices);
    size_t height       = q.remove_hook(hook_length, lsb_index);
    result += (((height % 2) == 0) ? -1 : 1) * MurnaghanNakayama(q, parts);
  }
  return result;
}

}  // namespace

SymmetricGroupCharacter SymmetricGroupCharacter::Irreducible(
    Partition const &p) {
  SymmetricGroupCharacter result;
  for (Partition partition : Partition::All(p.whole())) {
    int64_t value = MurnaghanNakayama(
        BoundaryPartition(p),
        absl::MakeConstSpan(&*partition.begin(), partition.parts()));
    if (value != 0) { result.values_.try_emplace(std::move(partition), value); }
  }
  return result;
}

SymmetricGroupCharacter SymmetricGroupCharacter::KroneckerDelta(
    Partition const &p) {
  SymmetricGroupCharacter result;
  result.values_.try_emplace(p, 1);
  return result;
}

SymmetricGroupCharacter &SymmetricGroupCharacter::operator+=(
    SymmetricGroupCharacter const &rhs) {
  for (auto const &[partition, coefficient] : rhs.values_) {
    auto [iter, inserted] = values_.try_emplace(partition, coefficient);
    if (not inserted) {
      if (iter->second == -coefficient) {
        values_.erase(iter);
      } else {
        iter->second += coefficient;
      }
    }
  }
  return *this;
}

SymmetricGroupCharacter &SymmetricGroupCharacter::operator-=(
    SymmetricGroupCharacter const &rhs) {
  for (auto const &[partition, coefficient] : rhs.values_) {
    auto [iter, inserted] = values_.try_emplace(partition, -coefficient);
    if (not inserted) {
      if (iter->second == coefficient) {
        values_.erase(iter);
      } else {
        iter->second -= coefficient;
      }
    }
  }
  return *this;
}

SymmetricGroupCharacter SymmetricGroupCharacter::operator-() const {
  SymmetricGroupCharacter result = *this;
  result *= -1;
  return result;
}

SymmetricGroupCharacter& SymmetricGroupCharacter::operator*=(int64_t n) {
  if (n == 0) {
    values_.clear();
  } else {
    for (auto &[partition, coefficient] : values_) { coefficient *= n; }
  }
  return *this;
}

}  // namespace chalk

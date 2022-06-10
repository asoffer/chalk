#include "integer.h"

#include <iostream>
#include "absl/numeric/int128.h"
#include "absl/strings/str_format.h"

namespace chalk {
namespace {

std::pair<uint64_t, uint64_t> Add(uint64_t l, uint64_t r) {
  return std::make_pair(l + r, l + r < l ? 1 : 0);
}

std::pair<uint64_t, uint64_t> Multiply(uint64_t l, uint64_t r) {
  absl::uint128 result = l;
  result *= r;
  return std::make_pair(absl::Uint128Low64(result),
                        absl::Uint128High64(result));
}

}  // namespace

Integer Integer::operator-() const {
  Integer result = *this;
  result.negate();
  return result;
}

Integer &Integer::AddStartingAt(uint64_t n, uintptr_t offset) {
  EnsureCapacity(std::max(size(), offset + 1) + 1);
  auto lhs_iter = std::next(begin(), offset);

  uint64_t carry = n;
  while (carry != 0) {
    uint64_t previous_carry    = carry;
    std::tie(*lhs_iter, carry) = Add(*lhs_iter, carry);
    ++lhs_iter;
    if (lhs_iter == end()) {
      IncrementSize();
      *lhs_iter = carry;
      break;
    }
  }
  return *this;
}

Integer &Integer::AddStartingAt(Integer const &rhs, uintptr_t offset) {
  EnsureCapacity(std::max(size(), rhs.size() + offset) + 1);
  auto lhs_iter = std::next(begin(), offset);
  auto rhs_end = rhs.end();

  bool carry = false;
  for (auto rhs_iter = rhs.begin(); rhs_iter != rhs_end;
       ++lhs_iter, ++rhs_iter) {
    uintptr_t previous_value = *lhs_iter;
    *lhs_iter += *rhs_iter + (carry ? 1 : 0);
    carry = (previous_value > *lhs_iter);
  }
  if (lhs_iter == end()) { IncrementSize(); }
  if (carry) { ++*lhs_iter; }
  return *this;
}

Integer &Integer::operator+=(Integer const &rhs) {
  return AddStartingAt(rhs, 0);
}

Integer &Integer::operator-=(Integer const &rhs) { return *this += -rhs; }

Integer operator*(Integer const &lhs, Integer const &rhs) {
  Integer result;
  result.EnsureCapacity(lhs.size() + rhs.size());
  for (auto lhs_iter = lhs.begin(); lhs_iter != lhs.end(); ++lhs_iter) {
    result.AddStartingAt(*lhs_iter * rhs, std::distance(lhs.begin(), lhs_iter));
  }

  return result;
}
void Integer::MultiplyBy(uint64_t n) {
  EnsureCapacity(size() + 1);
  uint64_t carry = 0;
  for (uint64_t &word : *this) {
    uint64_t previous_carry = carry;
    std::tie(word, carry)   = Multiply(n, word);
    word += previous_carry;
  }
  if (carry) {
    IncrementSize();
    back() = carry;
  }
}

std::ostream &operator<<(std::ostream &os, Integer const &n) {
  absl::Format(&os, "0x%x", *(n.end() - 1));
  for (auto iter = n.end() - 2; iter >= n.begin(); --iter) {
    absl::Format(&os, "%016x", *iter);
  }
  return os;
}

}  // namespace chalk

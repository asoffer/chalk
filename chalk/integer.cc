#include "integer.h"

#include <iostream>

#include "absl/numeric/int128.h"
#include "absl/strings/str_format.h"

namespace chalk {
namespace {

[[maybe_unused]] std::pair<uint64_t, uint64_t> Subtract(uint64_t l,
                                                        uint64_t r) {
  return std::make_pair(l - r, r > l ? 1 : 0);
}

[[maybe_unused]] std::pair<uint64_t, uint64_t> Add(uint64_t l, uint64_t r) {
  return std::make_pair(l + r, l + r < l ? 1 : 0);
}

std::pair<uint64_t, uint64_t> Multiply(uint64_t l, uint64_t r) {
  absl::uint128 result = l;
  result *= r;
  return std::make_pair(absl::Uint128Low64(result),
                        absl::Uint128High64(result));
}

}  // namespace

Integer::Integer(uint64_t n)
    : data_{reinterpret_cast<uintptr_t>(new uint64_t[1]), 1, 1} {
  span()[0] = n;
}

Integer::Integer(Integer const &n) {
  uint64_t *ptr = new uint64_t[n.data_[1]];
  data_[1]      = n.data_[1];
  data_[2]      = n.data_[1];
  std::memcpy(ptr, n.span().data(), span().size() * sizeof(uint64_t));
  data_[0] = reinterpret_cast<uintptr_t>(ptr) | (n.data_[0] & 1);
}

Integer::Integer(Integer &&n) {
  std::memcpy(data_, n.data_, sizeof(uintptr_t) * 3);
  std::memset(n.data_, 0, sizeof(uintptr_t) * 3);
}

Integer &Integer::operator=(Integer const &n) {
  this->~Integer();
  new (this) Integer(n);
  return *this;
}

Integer &Integer::operator=(Integer &&n) {
  std::memcpy(data_, n.data_, sizeof(uintptr_t) * 3);
  std::memset(n.data_, 0, sizeof(uintptr_t) * 3);
  return *this;
}

Integer::~Integer() { delete[] span().data(); }

Integer Integer::operator-() & {
  Integer result = *this;
  result.negate();
  return result;
}
Integer Integer::operator-() const & {
  Integer result = *this;
  result.negate();
  return result;
}

Integer Integer::operator-() && {
  negate();
  return std::move(*this);
}

Integer &Integer::SignSafeAddition(Integer const &rhs, uintptr_t offset) {
  EnsureCapacity(std::max(span().size(), rhs.span().size() + offset) + 1);
  auto lhs_iter = std::next(span().begin(), offset);
  auto rhs_end  = rhs.span().end();

  bool carry = false;
  for (auto rhs_iter = rhs.span().begin(); rhs_iter != rhs_end;
       ++lhs_iter, ++rhs_iter) {
    uintptr_t previous_value = *lhs_iter;
    *lhs_iter += *rhs_iter + (carry ? 1 : 0);
    carry = (previous_value > *lhs_iter);
  }
  if (lhs_iter == span().end()) { IncrementSize(); }
  if (carry) { ++*lhs_iter; }
  ShrinkToFit();
  return *this;
}

Integer &Integer::SignSafeSubtraction(Integer const &rhs, uintptr_t offset) {
  EnsureCapacity(std::max(span().size(), rhs.span().size() + offset) + 1);
  auto lhs_iter = std::next(span().begin(), offset);
  auto rhs_end  = rhs.span().end();
  bool carry = false;
  for (auto rhs_iter = rhs.span().begin(); rhs_iter != rhs_end;
       ++lhs_iter, ++rhs_iter) {
    uintptr_t previous_value = *lhs_iter;
    *lhs_iter -= *rhs_iter + (carry ? 1 : 0);
    carry = (previous_value < *rhs_iter);
  }
  if (carry) { --*lhs_iter; }
  ShrinkToFit();
  return *this;
}

Integer &Integer::operator+=(Integer const &rhs) {
  if (IsNegative(*this)) {
    negate();
    *this -= rhs;
    if (not IsZero()) { negate(); }
  } else {
    if (IsNegative(rhs)) {
      if (MagnitudeLess(*this, rhs)) {
        Integer result = rhs;
        result.SignSafeSubtraction(*this, 0);
        *this = std::move(result);
      } else {
        SignSafeSubtraction(rhs, 0);
      }
    } else {
      SignSafeAddition(rhs, 0);
    }
  }
  return *this;
}

Integer &Integer::operator-=(Integer const &rhs) {
  if (IsNegative(*this)) {
    negate();
    *this += rhs;
    if (not IsZero()) { negate(); }
  } else {
    if (IsNegative(rhs)) {
      SignSafeAddition(rhs, 0);
    } else {
      if (MagnitudeLess(*this, rhs)) {
        Integer result = rhs;
        result.SignSafeSubtraction(*this, 0);
        *this = std::move(result);
        negate();
      } else {
        SignSafeSubtraction(rhs, 0);
      }
    }
  }
  return *this;
}

Integer operator*(Integer const &lhs, Integer const &rhs) {
  Integer result;
  result.EnsureCapacity(lhs.span().size() + rhs.span().size());
  for (auto lhs_iter = lhs.span().begin(); lhs_iter != lhs.span().end();
       ++lhs_iter) {
    result.SignSafeAddition(*lhs_iter * rhs,
                            std::distance(lhs.span().begin(), lhs_iter));
  }

  if (Integer::IsNegative(lhs) != Integer::IsNegative(rhs)) { result.negate(); }
  result.ShrinkToFit();
  return result;
}

void Integer::MultiplyBy(uint64_t n) {
  EnsureCapacity(span().size() + 1);
  uint64_t carry = 0;
  for (uint64_t &word : span()) {
    uint64_t previous_carry = carry;
    std::tie(word, carry)   = Multiply(n, word);
    word += previous_carry;
  }
  if (carry) {
    IncrementSize();
    span().back() = carry;
  }
  ShrinkToFit();
}

std::ostream &operator<<(std::ostream &os, Integer const &n) {
  absl::Format(&os, "%s0x%x", Integer::IsNegative(n) ? "-" : "",
               *n.span().rbegin());
  for (auto iter = std::next(n.span().rbegin()); iter != n.span().rend();
       ++iter) {
    absl::Format(&os, "%016x", *iter);
  }
  return os;
}

void Integer::EnsureCapacity(size_t capacity) {
  if (data_[2] >= capacity) { return; }
  data_[2]      = std::max(2 * data_[2], capacity);
  uint64_t *ptr = new uint64_t[data_[2]];
  std::memcpy(ptr, span().data(), span().size() * sizeof(uint64_t));
  delete span().data();
  data_[0] = reinterpret_cast<uintptr_t>(ptr) | (data_[0] & 1);
}

bool Integer::IsZero() const {
  return span().size() == 1 and span().front() == 0;
}

void Integer::ShrinkToFit() {
  while (span().size() > 1 and span().back() == 0) { --data_[1]; }
  if (IsZero()) { data_[0] &= ~uintptr_t{1}; }
}

void Integer::IncrementSize() {
  EnsureCapacity(span().size() + 1);
  ++data_[1];
  span().back() = 0;
}

Integer operator/(Integer const &lhs, Integer const &rhs) {
  // TODO: Fully implement this.
  assert(lhs.span().size() == 1);
  assert(rhs.span().size() == 1);
  return lhs.span()[0] / rhs.span()[0];
}

bool operator==(Integer const &lhs, Integer const &rhs) {
  if (lhs.sign() != rhs.sign()) { return false; }
  if (lhs.span().size() != rhs.span().size()) { return false; }
  auto lhs_iter = lhs.span().begin();
  auto rhs_iter = rhs.span().begin();
  auto lhs_end  = lhs.span().end();
  for (; lhs_iter != lhs_end; ++lhs_iter, ++rhs_iter) {
    if (*lhs_iter != *rhs_iter) { return false; }
  }
  return true;
}

bool Integer::MagnitudeLess(Integer const &lhs, Integer const &rhs) {
  if (lhs.span().size() < rhs.span().size()) { return true; }
  if (lhs.span().size() > rhs.span().size()) { return false; }
  auto lhs_iter = lhs.span().end() - 1;
  auto rhs_iter = rhs.span().end() - 1;
  auto lhs_end  = lhs.span().begin() - 1;
  for (; lhs_iter != lhs_end; --lhs_iter, --rhs_iter) {
    if (*lhs_iter < *rhs_iter) { return not Integer::IsNegative(lhs); }
    if (*lhs_iter > *rhs_iter) { return Integer::IsNegative(lhs); }
  }
  return false;
}

bool operator<(Integer const &lhs, Integer const &rhs) {
  if (lhs.sign() < rhs.sign()) { return true; }
  if (lhs.sign() > rhs.sign()) { return false; }
  return Integer::MagnitudeLess(lhs, rhs);
}

}  // namespace chalk

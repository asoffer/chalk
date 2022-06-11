#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ostream>

#include "absl/types/span.h"

namespace chalk {

// A type sufficient for holding arbitrarily large integers (limited only by
// system memory constraints).
struct Integer {
  Integer(uint64_t n = 0);

  Integer(std::signed_integral auto n)
      : Integer(static_cast<uint64_t>(std::abs(n))) {
    if (n < 0) { negate(); }
  }

  Integer(std::unsigned_integral auto n) : Integer(static_cast<uint64_t>(n)) {}

  Integer(Integer const &n);
  Integer(Integer &&n);

  Integer &operator=(Integer const &n);
  Integer &operator=(Integer &&n);

  ~Integer();

  // Comparisons
  friend bool operator==(Integer const &lhs, Integer const &rhs);
  friend bool operator==(std::integral auto lhs, Integer const &rhs) {
    if (rhs.span().size() != 1) { return false; }
    if constexpr (std::signed_integral<decltype(lhs)>) {
      return ((lhs >= 0) ^ IsNegative(rhs)) and rhs.span()[0] == std::abs(lhs);
    } else {
      return not IsNegative(rhs) and rhs.span()[0] == lhs;
    }
  }
  friend bool operator==(Integer const &lhs, std::integral auto rhs) {
    return rhs == lhs;
  }

  friend bool operator!=(std::integral auto lhs, Integer const &rhs) {
    return not(lhs == rhs);
  }
  friend bool operator!=(Integer const &lhs, std::integral auto rhs) {
    return not(lhs == rhs);
  }
  friend bool operator!=(Integer const &lhs, Integer const &rhs) {
    return not(lhs == rhs);
  }

  friend bool operator<(Integer const &lhs, Integer const &rhs);

  friend bool operator<(std::integral auto lhs, Integer const &rhs) {
    if constexpr (std::signed_integral<decltype(lhs)>) {
      if (IsNegative(lhs)) {
        if (IsNegative(rhs)) {
          return rhs.span().size() == 1 and -lhs > rhs.span()[0];
        } else {
          return true;
        }
      } else {
        if (IsNegative(rhs)) {
          return false;
        } else {
          return rhs.span().size() > 1 or lhs < rhs.span()[0];
        }
      }
    } else {
      return rhs.span().size() > 1 or lhs < rhs.span()[0];
    }
  }

  friend bool operator<(Integer const &lhs, std::integral auto rhs) {
    if constexpr (std::signed_integral<decltype(rhs)>) {
      if (IsNegative(rhs)) {
        if (IsNegative(lhs)) {
          return lhs.span().size() > 1 or lhs.span()[0] > -rhs;
        } else {
          return false;
        }
      } else {
        if (IsNegative(lhs)) {
          return true;
        } else {
          return lhs.span().size() == 1 and lhs.span()[0] < rhs;
        }
      }
    } else {
      return lhs.span().size() > 1 or lhs.span()[0] > lhs;
    }
  }

  friend bool operator<=(Integer const &lhs, Integer const &rhs) {
    return not(rhs < lhs);
  }
  friend bool operator<=(std::integral auto lhs, Integer const &rhs) {
    return not(rhs < lhs);
  }
  friend bool operator<=(Integer const &lhs, std::integral auto rhs) {
    return not(rhs < lhs);
  }

  friend bool operator>(std::integral auto lhs, Integer const &rhs) {
    return rhs < lhs;
  }
  friend bool operator>(Integer const &lhs, std::integral auto rhs) {
    return rhs < lhs;
  }
  friend bool operator>(Integer const &lhs, Integer const &rhs) {
    return rhs < lhs;
  }

  friend bool operator>=(Integer const &lhs, Integer const &rhs) {
    return rhs <= lhs;
  }
  friend bool operator>=(std::integral auto lhs, Integer const &rhs) {
    return rhs <= lhs;
  }
  friend bool operator>=(Integer const &lhs, std::integral auto rhs) {
    return rhs <= lhs;
  }

  // Addition
  Integer &operator+=(Integer const &rhs);
  Integer &operator+=(std::integral auto rhs) {
    using int_t = std::conditional_t<std::signed_integral<decltype(rhs)>,
                                     int64_t, uint64_t>;
    return AddStartingAt(static_cast<int_t>(rhs), 0);
  }

  friend Integer operator+(Integer lhs, Integer const &rhs) {
    return lhs += rhs;
  }

  friend Integer operator+(Integer const &lhs, Integer &&rhs) {
    return rhs += lhs;
  }

  friend Integer operator+(std::integral auto lhs, Integer rhs) {
    return rhs += lhs;
  }

  friend Integer operator+(Integer lhs, std::integral auto rhs) {
    return lhs += rhs;
  }

  // Subtraction
  Integer &operator-=(Integer const &rhs);
  Integer &operator-=(std::integral auto rhs) {
    using int_t = std::conditional_t<std::signed_integral<decltype(rhs)>,
                                     int64_t, uint64_t>;
    return SubtractStartingAt(static_cast<int_t>(rhs), 0);
  }

  friend Integer operator-(Integer lhs, Integer const &rhs) {
    return lhs -= rhs;
  }

  friend Integer operator-(Integer const &lhs, Integer &&rhs) {
    rhs -= lhs;
    return -std::move(rhs);
  }

  friend Integer operator-(std::integral auto lhs, Integer rhs) {
    rhs -= lhs;
    return -std::move(rhs);
  }

  friend Integer operator-(Integer lhs, std::integral auto rhs) {
    lhs -= rhs;
    return lhs;
  }

  // Negation
  Integer operator-() const &;
  Integer operator-() &;
  Integer operator-() &&;
  void negate() { data_[0] ^= 1; }

  // Multiplication
  friend Integer operator*(Integer const &lhs, Integer const &rhs);

  friend Integer operator*(std::integral auto lhs, Integer rhs) {
    return rhs *= lhs;
  }

  friend Integer operator*(Integer const &lhs, std::integral auto rhs) {
    return lhs *= rhs;
  }

  Integer &operator*=(std::integral auto n) {
    if constexpr (std::is_signed_v<decltype(n)>) {
      if (n < 0) {
        negate();
        n *= -1;
      }
    }
    MultiplyBy(static_cast<std::make_unsigned_t<decltype(n)>>(n));
    return *this;
  }

  Integer &operator*=(Integer const &rhs) {
    *this = *this * rhs;
    return *this;
  }

  // Division operations
  friend Integer operator/(Integer const &lhs, Integer const &rhs);
  Integer &operator/=(Integer const &rhs) {
    *this = *this / rhs;
    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os, Integer const &n);

  static bool IsNegative(std::signed_integral auto n) { return n < 0; }
  static bool IsNegative(Integer const &n) { return n.sign() < 0; }

 private:
  int sign() const { return data_[0] & 1 ? -1 : 1; }

  absl::Span<uint64_t> span() {
    return absl::MakeSpan(
        reinterpret_cast<uint64_t *>(data_[0] & ~uintptr_t{1}), data_[1]);
  }
  absl::Span<uint64_t const> span() const {
    return absl::MakeConstSpan(
        reinterpret_cast<uint64_t *>(data_[0] & ~uintptr_t{1}), data_[1]);
  }

  void EnsureCapacity(size_t capacity);
  void ShrinkToFit();
  void IncrementSize();

  void MultiplyBy(uint64_t n);
  Integer &AddStartingAt(Integer const &, uintptr_t);
  Integer &AddStartingAt(uint64_t, uintptr_t);
  Integer &AddStartingAt(int64_t, uintptr_t);
  Integer &SubtractStartingAt(uint64_t, uintptr_t);
  Integer &SubtractStartingAt(int64_t, uintptr_t);

  int sign_;
  uintptr_t data_[3];
};

}  // namespace chalk

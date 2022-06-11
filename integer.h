#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ostream>

namespace chalk {

// A type sufficient for holding arbitrarily large integers (limited only by
// system memory constraints).
struct Integer {
  Integer() : Integer(uint64_t{0}) {}

  Integer(uint64_t n)
      : data_{reinterpret_cast<uintptr_t>(new uint64_t[1]), 1, 1} {
    *data() = n;
  }

  template <std::integral T, std::enable_if_t<std::is_signed_v<T>, int> = 0>
  Integer(T n) : Integer(static_cast<uint64_t>(n < 0 ? -n : n)) {
    if (n < 0) { negate(); }
  }

  template <std::integral T, std::enable_if_t<std::is_unsigned_v<T>, int> = 0>
  Integer(T n) : Integer(n) {}

  Integer(Integer const &n) {
    uint64_t *ptr = new uint64_t[n.data_[1]];
    data_[1]      = n.data_[1];
    data_[2]      = n.data_[1];
    std::memcpy(ptr, n.data(), size() * sizeof(uint64_t));
    data_[0] = reinterpret_cast<uintptr_t>(ptr);
  }
  Integer(Integer &&n) {
    std::memcpy(data_, n.data_, sizeof(uintptr_t) * 3);
    std::memset(n.data_, 0, sizeof(uintptr_t) * 3);
  }
  ~Integer() { delete[] data(); }

  Integer &operator=(Integer const &n) {
    this->~Integer();
    new (this) Integer(n);
    return *this;
  }

  Integer &operator=(Integer &&n) {
    std::memcpy(data_, n.data_, sizeof(uintptr_t) * 3);
    std::memset(n.data_, 0, sizeof(uintptr_t) * 3);
    return *this;
  }

  // Addition operations
  Integer &operator+=(Integer const &rhs);

  Integer &operator+=(std::integral auto rhs) { return AddStartingAt(rhs, 0); }

  friend Integer operator+(Integer lhs, Integer const &rhs) {
    return lhs += rhs;
  }

  // Subtraction operations
  Integer &operator-=(Integer const &rhs);

  friend Integer operator-(Integer lhs, Integer const &rhs) {
    return lhs -= rhs;
  }

  Integer operator-() const;

  void negate() { data_[0] ^= 1; }

  // Multiplication operations
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

  // Comparisons
  friend bool operator==(Integer const &lhs, Integer const &rhs) {
    if (lhs.sign() != rhs.sign()) { return false; }
    if (lhs.size() != rhs.size()) { return false; }
    auto lhs_iter = lhs.begin();
    auto rhs_iter = rhs.begin();
    auto lhs_end  = lhs.end();
    for (; lhs_iter != lhs_end; ++lhs_iter, ++rhs_iter) {
      if (*lhs_iter != *rhs_iter) { return false; }
    }
    return true;
  }
  friend bool operator==(std::integral auto lhs, Integer const &rhs) {
    // TODO: Improve implementation.
    return Integer(lhs) == rhs;
  }
  friend bool operator==(Integer const &lhs, std::integral auto rhs) {
    // TODO: Improve implementation.
    return lhs == Integer(rhs);
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
  friend bool operator<(Integer const &lhs, Integer const &rhs) {
    if (lhs.sign() < rhs.sign()) { return true; }
    if (lhs.sign() > rhs.sign()) { return false; }
    if (lhs.size() < rhs.size()) { return true; }
    if (lhs.size() > rhs.size()) { return false; }
    auto lhs_iter = lhs.end() - 1;
    auto rhs_iter = rhs.end() - 1;
    auto lhs_end  = lhs.begin() - 1;
    for (; lhs_iter != lhs_end; --lhs_iter, --rhs_iter) {
      if (*lhs_iter < *rhs_iter) { return lhs.positive(); }
      if (*lhs_iter > *rhs_iter) { return lhs.negative(); }
    }
    return false;
  }

  friend bool operator<=(Integer const &lhs, Integer const &rhs) {
    return not(rhs < lhs);
  }
  friend bool operator>(Integer const &lhs, Integer const &rhs) {
    return rhs < lhs;
  }
  friend bool operator>=(Integer const &lhs, Integer const &rhs) {
    return not(rhs < lhs);
  }

  template <typename H>
  friend H AbslHashValue(H h, Integer const &n);

  friend std::ostream &operator<<(std::ostream &os, Integer const &n);

 private:
  void MultiplyBy(uint64_t n);
  int sign() const { return data_[0] & 1 ? 1 : -1; }
  bool positive() const { return sign() > 0; }
  bool negative() const { return sign() < 0; }

  uint64_t *data() {
    return reinterpret_cast<uint64_t *>(data_[0] & ~uintptr_t{1});
  }
  uint64_t const *data() const {
    return reinterpret_cast<uint64_t *>(data_[0] & ~uintptr_t{1});
  }
  uint64_t *begin() { return data(); }
  uint64_t const *begin() const { return data(); };
  uint64_t *end() { return begin() + size(); }
  uint64_t const *end() const { return begin() + size(); }

  uint64_t &back() { return *(end() - 1); }

  size_t size() const { return data_[1]; }

  void EnsureCapacity(size_t capacity) {
    if (data_[2] >= capacity) { return; }
    data_[2]      = std::max(2 * data_[2], capacity);
    uint64_t *ptr = new uint64_t[data_[2]];
    std::memcpy(ptr, data(), size() * sizeof(uint64_t));
    delete data();
    data_[0] = reinterpret_cast<uintptr_t>(ptr);
  }
  void ShrinkToFit();
  void IncrementSize() {
    EnsureCapacity(size() + 1);
    *(data() + data_[1]) = 0;
    ++data_[1];
  }

  Integer &AddStartingAt(Integer const &, uintptr_t);
  Integer &AddStartingAt(uint64_t, uintptr_t);

  int sign_;
  uintptr_t data_[3];
};

}  // namespace chalk

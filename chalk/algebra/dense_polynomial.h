#ifndef CHALK_ALGEBRA_DENSE_POLYNOMIAL_H
#define CHALK_ALGEBRA_DENSE_POLYNOMIAL_H

#include <cstddef>
#include <initializer_list>
#include <vector>

#include "absl/types/span.h"
#include "chalk/algebra/property.h"

namespace chalk {

template <Satisfies<Ring> R>
struct DensePolynomial : Algebraic {
  using chalk_properties =
      void(Ring, std::conditional_t<Satisfies<R, Commutative<'*'>>,
                                    Commutative<'*'>, Ring>);
  using coefficient_ring = R;

  // Constructs the zero polynomial.
  explicit DensePolynomial() = default;

  static DensePolynomial Zero() { return DensePolynomial(); }

  static DensePolynomial One() {
    return DensePolynomial(Unit<'*', coefficient_ring>());
  }

  // Constructs the constant polynomial equal to `element`.
  template <std::convertible_to<coefficient_ring> T>
  DensePolynomial(T &&element) {
    if (not IsUnit<'+'>(element)) {
      coefficients_.push_back(std::forward<T>(element));
    }
  }

  // Constructs the polynomial equivalent to `p` with each coefficient converted
  // to `coefficient_ring`.
  template <std::convertible_to<coefficient_ring> T>
  DensePolynomial(DensePolynomial<T> const &p) requires(
      not std::is_same_v<T, coefficient_ring>)
      : coefficients_(p.coefficients_.begin(), p.coefficients_.end()) {
    Compact();
  }

  // Constructs a polynomial whose coefficients are given by the sequnece
  // `[begin, end)`
  static DensePolynomial FromCoefficients(auto begin, auto end) {
    DensePolynomial p;
    p.coefficients_.assign(begin, end);
    p.Compact();
    return p;
  }

  // Constructs a polynomial whose coefficients are given by the sequnece
  // `coefficients`
  static DensePolynomial FromCoefficients(
      std::initializer_list<coefficient_ring> coefficients) {
    return FromCoefficients(coefficients.begin(), coefficients.end());
  }

  // Returns the degree of the polynomial if it is non-zero and `-1` otherwise.
  int64_t degree() const {
    return static_cast<int64_t>(coefficients_.size()) - 1;
  }

  absl::Span<coefficient_ring const> coefficients() const {
    return coefficients_;
  }

  // Returns the coefficient of the monomial of degree `d`, or zero of no such
  // monomial exists.
  coefficient_ring const &coefficient(size_t d) const {
    return static_cast<int64_t>(d) > degree() ? Unit<'+', coefficient_ring>()
                                              : coefficients_[d];
  }

  template <std::convertible_to<DensePolynomial> T>
  DensePolynomial &operator+=(T &&v) {
    using type = std::decay_t<T>;
    if constexpr (std::is_same_v<DensePolynomial, type>) {
      size_t p_size    = v.coefficients_.size();
      size_t self_size = coefficients_.size();
      if (p_size > self_size) { coefficients_.resize(p_size); }
      for (size_t i = 0; i < p_size; ++i) {
        coefficients_[i] += v.coefficients_[i];
      }
      Compact();

      return *this;
    } else if constexpr (std::convertible_to<coefficient_ring, type>) {
      if (IsUnit<'+'>(v)) { return *this; }
      if (degree() < 0) {
        coefficients_.push_back(std::forward<T>(v));
      } else {
        auto &coef = coefficients_.front();
        coef += std::forward<T>(v);
        if (degree() == 0 and IsUnit<'+'>(coef)) { coefficients_.pop_back(); }
      }
      return *this;
    } else {
      return *this += DensePolynomial(std::forward<T>(v));
    }
  }

  template <std::convertible_to<DensePolynomial> T>
  DensePolynomial &operator-=(T &&v) {
    using type = std::decay_t<T>;
    if constexpr (std::is_same_v<DensePolynomial, type>) {
      size_t p_size    = v.coefficients_.size();
      size_t self_size = coefficients_.size();
      if (p_size > self_size) { coefficients_.resize(p_size); }
      for (size_t i = 0; i < p_size; ++i) {
        coefficients_[i] -= v.coefficients_[i];
      }
      Compact();
      return *this;
    } else if constexpr (std::convertible_to<coefficient_ring, type>) {
      if (IsUnit<'+'>(v)) { return *this; }
      if (degree() < 0) {
        coefficients_.push_back(-std::forward<T>(v));
      } else {
        auto &coef = coefficients_.front();
        coef -= std::forward<T>(v);
        if (degree() == 0 and IsUnit<'+'>(coef)) { coefficients_.pop_back(); }
      }
      return *this;
    } else {
      return *this -= DensePolynomial(std::forward<T>(v));
    }
  }

  template <std::convertible_to<DensePolynomial> T>
  DensePolynomial &operator*=(T &&v) {
    using type = std::decay_t<T>;
    if constexpr (std::is_same_v<DensePolynomial, type>) {
      switch (v.degree()) {
        case -1: coefficients_.clear(); break;
        case 0: return *this *= v.coefficients_[0];
        default: {
          auto lhs = std::exchange(coefficients_, {});
          coefficients_.resize(lhs.size() + v.degree(), 0);
          for (size_t i = 0; i < coefficients_.size(); ++i) {
            size_t v_size = v.coefficients_.size();
            size_t start  = (i + 1 < v_size ? 0 : i - v_size + 1);
            size_t end    = std::min(lhs.size(), i + 1);

            for (size_t j = start; j < end; ++j) {
              coefficients_[i] += lhs[j] * v.coefficients_[i - j];
            }
          }
          Compact();
        }
      }

      return *this;
    } else if constexpr (std::convertible_to<type, coefficient_ring>) {
      if (IsUnit<'+'>(v)) {
        coefficients_.clear();
      } else if (not IsUnit<'*'>(v)) {
        for (auto &c : coefficients_) { c *= v; }
      }
      return *this;
    } else {
      return *this *= DensePolynomial(std::forward<T>(v));
    }
  }

  friend bool operator==(std::convertible_to<DensePolynomial> auto const &lhs,
                         std::convertible_to<DensePolynomial> auto const &rhs) {
    using lhs_type = std::decay_t<decltype(lhs)>;
    using rhs_type = std::decay_t<decltype(rhs)>;
    if constexpr (not std::is_same_v<lhs_type, DensePolynomial>) {
      return rhs == lhs;
    } else {
      static_assert(std::is_same_v<lhs_type, DensePolynomial>);
      if constexpr (std::convertible_to<rhs_type, coefficient_ring>) {
        return lhs.degree() <= 0 and lhs.coefficient(0) == rhs;
      } else if constexpr (std::is_same_v<rhs_type, DensePolynomial>) {
        return lhs.coefficients_ == rhs.coefficients_;
      } else {
        return lhs == static_cast<DensePolynomial>(rhs);
      }
    }
  }

 private:
  template <Satisfies<Ring>>
  friend struct DensePolynomial;

  void Compact() {
    coefficients_.resize(std::distance(
        std::find_if(
            coefficients_.rbegin(), coefficients_.rend(),
            [](coefficient_ring const &coef) { return not IsUnit<'+'>(coef); }),
        coefficients_.rend()));
  }

  std::vector<coefficient_ring> coefficients_;
};

}  // namespace chalk

#endif  // CHALK_ALGEBRA_DENSE_POLYNOMIAL_H

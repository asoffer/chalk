#ifndef CHALK_ALGEBRA_MONOMIAL_H
#define CHALK_ALGEBRA_MONOMIAL_H

#include <concepts>
#include <cstddef>
#include <tuple>

#include "chalk/algebra/property.h"
#include "chalk/algebra/variable.h"

namespace chalk {

template <size_t N, typename E = uint16_t>
struct Monomial : Algebraic {
  using chalk_properties = void(Monoid<'*'>, Commutative<'*'>);

  using exponent_type                    = E;
  static constexpr size_t variable_count = N;

  static auto Variables() {
    return internal_variable::VariableSet<variable_count>();
  }

  // Constructs the multiplicative unit for the monomial.
  constexpr explicit Monomial() : exponents_{} {}

  // Constructs the multiplicative unit for the monomial.
  static constexpr Monomial One() { return Monomial(); }

  // Constructs a monomial consisting of the single variable `v`.
  template <size_t M>
  constexpr Monomial(Variable<M>) requires(M < variable_count) : exponents_{} {
    exponents_[M] = 1;
  }

  template <size_t M>
  constexpr Monomial(Monomial<M> const &m) requires(M < variable_count)
      : exponents_{} {
    for (size_t i = 0; i < M; ++i) { exponents_[i] = m.exponents_[i]; }
  }

  // Returns the exponent of the variable `v` in the monomial.
  template <size_t M>
  constexpr exponent_type const &exponent(Variable<M>) const
      requires(M < variable_count) {
    return exponents_[M];
  }

  template <typename H>
  friend H AbslHashValue(H h, Monomial const &m) {
    return H::combine_contiguous(std::move(h), &m.exponents_[0],
                                 variable_count);
  }

  Monomial &operator*=(Monomial const &m) {
    for (size_t i = 0; i < variable_count; ++i) {
      exponents_[i] += m.exponents_[i];
    }
    return *this;
  }

  template <size_t M>
  Monomial &operator*=(Variable<M>) requires(M < variable_count) {
    ++exponents_[M];
    return *this;
  }

  friend bool operator==(std::convertible_to<Monomial> auto const &lhs,
                         std::convertible_to<Monomial> auto const &rhs) {
    // TODO: Implement this more efficiently when you know that one side is a
    // smaller monomial, or a specific variable.
    Monomial l(lhs);
    Monomial r(rhs);
    for (size_t i = 0; i < variable_count; ++i) {
      if (l.exponents_[i] != r.exponents_[i]) { return false; }
    }
    return true;
  }

 private:
  template <size_t, typename>
  friend struct Monomial;

  exponent_type exponents_[variable_count];
};

}  // namespace chalk

#endif  // CHALK_ALGEBRA_MONOMIAL_H

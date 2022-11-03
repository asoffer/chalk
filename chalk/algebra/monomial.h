#ifndef CHALK_ALGEBRA_MONOMIAL_H
#define CHALK_ALGEBRA_MONOMIAL_H

#include <cstddef>

#include "chalk/algebra/property.h"

namespace chalk {

struct Variable {
  constexpr explicit Variable(size_t index) : index_(index) {}

  constexpr size_t index() const { return index_; }

 private:
  size_t index_;
};

template <size_t N, typename E = uint16_t>
struct Monomial : Algebraic {
  using chalk_properties = void(MonoidOver<'*'>, Commutative<'*'>);

  using exponent_type                    = E;
  static constexpr size_t variable_count = N;

  constexpr explicit Monomial() : exponents_{} {}
  constexpr Monomial(Variable v) : exponents_{} { exponents_[v.index()] = 1; }

  constexpr exponent_type const &exponent(Variable v) const {
    return exponents_[v.index()];
  }

  friend bool operator==(Monomial const &lhs, Monomial const &rhs) = default;
  friend bool operator!=(Monomial const &lhs, Monomial const &rhs) = default;

  Monomial &operator*=(Monomial const &m) {
    for (size_t i = 0; i < variable_count; ++i) {
      exponents_[i] += m.exponents_[i];
    }
    return *this;
  }

  Monomial &operator*=(Variable v) {
    ++exponents_[v.index()];
    return *this;
  }

 private:
  exponent_type exponents_[variable_count];
};

}  // namespace chalk

#endif  // CHALK_ALGEBRA_MONOMIAL_H

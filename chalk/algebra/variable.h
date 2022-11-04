#ifndef CHALK_ALGEBRA_VARIABLE_H
#define CHALK_ALGEBRA_VARIABLE_H

#include <cstddef>

namespace chalk {

// Represents a variable to be used in monomials, polynomials, etc.
struct Variable {
  constexpr explicit Variable(size_t index) : index_(index) {}

  constexpr size_t index() const { return index_; }

  friend bool operator==(Variable, Variable) = default;
  friend bool operator!=(Variable, Variable) = default;

 private:
  size_t index_;
};

}  // namespace chalk

#endif  // CHALK_ALGEBRA_VARIABLE_H

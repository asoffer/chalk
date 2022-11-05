#ifndef CHALK_ALGEBRA_VARIABLE_H
#define CHALK_ALGEBRA_VARIABLE_H

#include <cstddef>
#include <tuple>

namespace chalk {

// Represents a variable to be used in monomials, polynomials, etc.
template <size_t N>
struct Variable {
  constexpr static size_t index() { return N; }
};

namespace internal_variable {

template <size_t N>
struct VariableSet {
  template <size_t M>
  constexpr Variable<M> get() const requires(M < N) {
    return Variable<M>();
  }
};

}  // namespace internal_variable

template <size_t L, size_t R>
bool operator==(Variable<L>, Variable<R>) {
  return L == R;
}
template <size_t L, size_t R>
bool operator!=(Variable<L>, Variable<R>) {
  return L != R;
}

}  // namespace chalk

template <size_t Count>
struct ::std::tuple_size<chalk::internal_variable::VariableSet<Count>>
    : ::std::integral_constant<size_t, Count> {};

template <size_t N, size_t Count>
requires(N < Count) struct ::std::tuple_element<
    N, chalk::internal_variable::VariableSet<Count>> {
  using type = ::chalk::Variable<N>;
};

#endif  // CHALK_ALGEBRA_VARIABLE_H

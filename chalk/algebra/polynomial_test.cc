#include "chalk/algebra/polynomial.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {
namespace {

TEST(Polynomial, DefaultAndUnitConstruction) {
  using polynomial_type = Polynomial<int64_t, 3>;
  polynomial_type p;
  EXPECT_EQ(Unit<'+'>(), p);
  EXPECT_NE(Unit<'*'>(), p);

  EXPECT_EQ(Unit<'+'>(), polynomial_type::Zero());
  EXPECT_NE(Unit<'*'>(), polynomial_type::Zero());

  EXPECT_NE(Unit<'+'>(), polynomial_type::One());
  EXPECT_EQ(Unit<'*'>(), polynomial_type::One());

  EXPECT_EQ(polynomial_type::One().term_count(), 1);
  EXPECT_EQ(polynomial_type::Zero().term_count(), 0);
}

TEST(Polynomial, VariableConstruction) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();

  polynomial_type p = x;
  EXPECT_EQ(p.term_count(), 1);
  EXPECT_EQ(p.coefficient(x), 1);
  EXPECT_EQ(p.coefficient(y), 0);
  EXPECT_EQ(p.coefficient(z), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);
}

TEST(Polynomial, MonomialConstruction) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();

  polynomial_type p = monomial_type(x) * y;
  EXPECT_EQ(p.term_count(), 1);
  EXPECT_EQ(p.coefficient(x), 0);
  EXPECT_EQ(p.coefficient(y), 0);
  EXPECT_EQ(p.coefficient(z), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 1);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);
}

TEST(Polynomial, VariableAdditionAndSubstraction) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();

  polynomial_type p;

  p += x;
  EXPECT_EQ(p.term_count(), 1);
  EXPECT_EQ(p.coefficient(x), 1);
  EXPECT_EQ(p.coefficient(y), 0);
  EXPECT_EQ(p.coefficient(z), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);

  p += y;
  EXPECT_EQ(p.term_count(), 2);
  EXPECT_EQ(p.coefficient(x), 1);
  EXPECT_EQ(p.coefficient(y), 1);
  EXPECT_EQ(p.coefficient(z), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);

  p += x;
  EXPECT_EQ(p.term_count(), 2);
  EXPECT_EQ(p.coefficient(x), 2);
  EXPECT_EQ(p.coefficient(y), 1);
  EXPECT_EQ(p.coefficient(z), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);

  p -= z;
  EXPECT_EQ(p.term_count(), 3);
  EXPECT_EQ(p.coefficient(x), 2);
  EXPECT_EQ(p.coefficient(y), 1);
  EXPECT_EQ(p.coefficient(z), -1);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);

  p -= y;
  EXPECT_EQ(p.term_count(), 2);
  EXPECT_EQ(p.coefficient(x), 2);
  EXPECT_EQ(p.coefficient(y), 0);
  EXPECT_EQ(p.coefficient(z), -1);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);
}

TEST(Polynomial, MonomialAdditionAndSubstraction) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();

  polynomial_type p;

  p += monomial_type(x);
  EXPECT_EQ(p.term_count(), 1);
  EXPECT_EQ(p.coefficient(x), 1);
  EXPECT_EQ(p.coefficient(y), 0);
  EXPECT_EQ(p.coefficient(z), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);

  p += monomial_type(x) * y;
  EXPECT_EQ(p.term_count(), 2);
  EXPECT_EQ(p.coefficient(x), 1);
  EXPECT_EQ(p.coefficient(y), 0);
  EXPECT_EQ(p.coefficient(z), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 1);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);

  p += monomial_type(x);
  EXPECT_EQ(p.term_count(), 2);
  EXPECT_EQ(p.coefficient(x), 2);
  EXPECT_EQ(p.coefficient(y), 0);
  EXPECT_EQ(p.coefficient(z), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 1);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);

  p -= monomial_type(y);
  EXPECT_EQ(p.term_count(), 3);
  EXPECT_EQ(p.coefficient(x), 2);
  EXPECT_EQ(p.coefficient(y), -1);
  EXPECT_EQ(p.coefficient(z), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 1);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);

  p -= monomial_type(x) * y;
  EXPECT_EQ(p.term_count(), 2);
  EXPECT_EQ(p.coefficient(x), 2);
  EXPECT_EQ(p.coefficient(y), -1);
  EXPECT_EQ(p.coefficient(z), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * y), 0);
  EXPECT_EQ(p.coefficient(monomial_type(x) * x), 0);
}

TEST(Polynomial, CoefficientManiuplation) {
  using polynomial_type = Polynomial<int64_t, 3>;
  polynomial_type p;

  p += 3;
  EXPECT_EQ(p.term_count(), 1);
  EXPECT_EQ(p.coefficient(Unit<'*'>()), 3);

  p *= 3;
  EXPECT_EQ(p.term_count(), 1);
  EXPECT_EQ(p.coefficient(Unit<'*'>()), 9);

  p -= 10;
  EXPECT_EQ(p.term_count(), 1);
  EXPECT_EQ(p.coefficient(Unit<'*'>()), -1);

  p += 1;
  EXPECT_EQ(p.term_count(), 0);
  EXPECT_EQ(p.coefficient(Unit<'*'>()), 0);
}

// TODO: A lot more tests.

}  // namespace
}  // namespace chalk

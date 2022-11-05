#include "chalk/algebra/monomial.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {
namespace {

TEST(Monomial, DefaultConstruction) {
  using monomial_type = Monomial<8>;
  monomial_type m;
  for (size_t i = 0; i < monomial_type::variable_count; ++i) {
    EXPECT_EQ(m.exponent(Variable<1>()), 0);
  }

  EXPECT_EQ(Unit<'*'>(), m);
}

TEST(Monomial, VariableConstruction) {
  using monomial_type = Monomial<4>;
  monomial_type m     = Variable<2>();
  EXPECT_EQ(m.exponent(Variable<0>()), 0);
  EXPECT_EQ(m.exponent(Variable<1>()), 0);
  EXPECT_EQ(m.exponent(Variable<2>()), 1);
  EXPECT_EQ(m.exponent(Variable<3>()), 0);
}

TEST(Monomial, MultiplicationByVariable) {
  using monomial_type = Monomial<4>;
  monomial_type m     = Variable<2>();
  m *= Variable<1>();
  EXPECT_EQ(m.exponent(Variable<0>()), 0);
  EXPECT_EQ(m.exponent(Variable<1>()), 1);
  EXPECT_EQ(m.exponent(Variable<2>()), 1);
  EXPECT_EQ(m.exponent(Variable<3>()), 0);
  m = m * Variable<2>();
  EXPECT_EQ(m.exponent(Variable<0>()), 0);
  EXPECT_EQ(m.exponent(Variable<1>()), 1);
  EXPECT_EQ(m.exponent(Variable<2>()), 2);
  EXPECT_EQ(m.exponent(Variable<3>()), 0);
}

TEST(Monomial, MultiplicationByMonomial) {
  using monomial_type = Monomial<4>;
  monomial_type m1    = Variable<1>();
  monomial_type m2    = Variable<2>();
  m1 *= m2;
  EXPECT_EQ(m1.exponent(Variable<0>()), 0);
  EXPECT_EQ(m1.exponent(Variable<1>()), 1);
  EXPECT_EQ(m1.exponent(Variable<2>()), 1);
  EXPECT_EQ(m1.exponent(Variable<3>()), 0);
  m1 = m1 * m2;
  EXPECT_EQ(m1.exponent(Variable<0>()), 0);
  EXPECT_EQ(m1.exponent(Variable<1>()), 1);
  EXPECT_EQ(m1.exponent(Variable<2>()), 2);
  EXPECT_EQ(m1.exponent(Variable<3>()), 0);
}

TEST(Monomial, Variables) {
  using monomial_type = Monomial<3>;
  auto [x, y, z]      = monomial_type::Variables();
  EXPECT_EQ(x, Variable<0>());
  EXPECT_EQ(y, Variable<1>());
  EXPECT_EQ(z, Variable<2>());
}

TEST(Monomial, Equality) {
  using monomial_type = Monomial<3>;
  auto [x, y, z]      = monomial_type::Variables();
  monomial_type m1    = monomial_type(x) * x * y;
  monomial_type m2    = monomial_type(x) * y * x;
  monomial_type m3    = monomial_type(x) * y * z;
  EXPECT_TRUE(m1 == m2);
  EXPECT_FALSE(m1 != m2);
  EXPECT_FALSE(m1 == m3);
  EXPECT_TRUE(m1 != m3);
}

TEST(Monomial, Conversion) {
  auto [x] = Monomial<1>::Variables();
  auto [x2, y, z] = Monomial<3>::Variables();
  EXPECT_EQ(x, x2);

  Monomial<1> m1 = x;
  Monomial<3> m3 = x;
  EXPECT_EQ(m1, m3);
  EXPECT_EQ(m1, x);
  EXPECT_EQ(m3, x);
}

}  // namespace
}  // namespace chalk

#include "chalk/algebra/monomial.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {
namespace {

TEST(Monomial, DefaultConstruction) {
  using monomial_type = Monomial<8>;
  monomial_type m;
  for (size_t i = 0; i < monomial_type::variable_count; ++i) {
    EXPECT_EQ(m.exponent(Variable(1)), 0);
  }
}

TEST(Monomial, VariableConstruction) {
  using monomial_type = Monomial<4>;
  monomial_type m     = Variable(2);
  EXPECT_EQ(m.exponent(Variable(0)), 0);
  EXPECT_EQ(m.exponent(Variable(1)), 0);
  EXPECT_EQ(m.exponent(Variable(2)), 1);
  EXPECT_EQ(m.exponent(Variable(3)), 0);
}

TEST(Monomial, MultiplicationByVariable) {
  using monomial_type = Monomial<4>;
  monomial_type m     = Variable(2);
  m *= Variable(1);
  EXPECT_EQ(m.exponent(Variable(0)), 0);
  EXPECT_EQ(m.exponent(Variable(1)), 1);
  EXPECT_EQ(m.exponent(Variable(2)), 1);
  EXPECT_EQ(m.exponent(Variable(3)), 0);
  m = m * Variable(2);
  EXPECT_EQ(m.exponent(Variable(0)), 0);
  EXPECT_EQ(m.exponent(Variable(1)), 1);
  EXPECT_EQ(m.exponent(Variable(2)), 2);
  EXPECT_EQ(m.exponent(Variable(3)), 0);
}

TEST(Monomial, MultiplicationByMonomial) {
  using monomial_type = Monomial<4>;
  monomial_type m1    = Variable(1);
  monomial_type m2    = Variable(2);
  m1 *= m2;
  EXPECT_EQ(m1.exponent(Variable(0)), 0);
  EXPECT_EQ(m1.exponent(Variable(1)), 1);
  EXPECT_EQ(m1.exponent(Variable(2)), 1);
  EXPECT_EQ(m1.exponent(Variable(3)), 0);
  m1 = m1 * m2;
  EXPECT_EQ(m1.exponent(Variable(0)), 0);
  EXPECT_EQ(m1.exponent(Variable(1)), 1);
  EXPECT_EQ(m1.exponent(Variable(2)), 2);
  EXPECT_EQ(m1.exponent(Variable(3)), 0);
}

}  // namespace
}  // namespace chalk

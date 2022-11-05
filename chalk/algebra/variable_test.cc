#include "chalk/algebra/variable.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {
namespace {

TEST(Variable, Construction) {
  Variable<3> v;
  EXPECT_EQ(v.index(), 3);

  EXPECT_TRUE(v == Variable<3>());
  EXPECT_FALSE(v != Variable<3>());

  EXPECT_FALSE(v == Variable<4>());
  EXPECT_TRUE(v != Variable<4>());
}

TEST(VariableSet, StructuredBinding) {
  auto [x, y, z] = internal_variable::VariableSet<3>();
  EXPECT_EQ(x, Variable<0>());
  EXPECT_EQ(y, Variable<1>());
  EXPECT_EQ(z, Variable<2>());
}

}  // namespace
}  // namespace chalk

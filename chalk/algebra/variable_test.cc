#include "chalk/algebra/variable.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {
namespace {

TEST(Variable, Construction) {
  Variable v(3);
  EXPECT_EQ(v.index(), 3);

  EXPECT_TRUE(v == Variable(3));
  EXPECT_FALSE(v != Variable(3));

  EXPECT_FALSE(v == Variable(4));
  EXPECT_TRUE(v != Variable(4));
}

}  // namespace
}  // namespace chalk

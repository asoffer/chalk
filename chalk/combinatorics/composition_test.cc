#include "chalk/combinatorics/composition.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {

using ::testing::ElementsAre;

TEST(Composition, InitializerListConstruction) {
  Composition c{2, 3, 2, 4};
  EXPECT_EQ(c.parts(), 4);
  EXPECT_EQ(c.whole(), 11);
  EXPECT_THAT(c, ElementsAre(2, 3, 2, 4));
  EXPECT_EQ(c[0], 2);
  EXPECT_EQ(c[1], 3);
  EXPECT_EQ(c[2], 2);
  EXPECT_EQ(c[3], 4);
}

TEST(Composition, OrderDependent) {
  Composition c{2, 5, 2, 1};
  EXPECT_NE(c, (Composition{5, 1, 2, 2}));
  EXPECT_NE(c, (Composition{1, 2, 5, 2}));
}

}  // namespace chalk

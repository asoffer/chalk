#include "combinatorics/partition.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {

using ::testing::ElementsAre;

TEST(Partition, InitializerListConstruction) {
  Partition p{5, 2, 2, 1};
  EXPECT_EQ(p.parts(), 4);
  EXPECT_EQ(p.whole(), 10);
  EXPECT_THAT(p, ElementsAre(5, 2, 2, 1));
  EXPECT_EQ(p[0], 5);
  EXPECT_EQ(p[1], 2);
  EXPECT_EQ(p[2], 2);
  EXPECT_EQ(p[3], 1);
}

TEST(Partition, Trivial) {
  Partition p = Partition::Trivial();
  EXPECT_EQ(p.parts(), 0);
  EXPECT_EQ(p.whole(), 0);
  EXPECT_THAT(p, ElementsAre());
}

TEST(Partition, Full) {
  Partition p = Partition::Full(5);
  EXPECT_EQ(p.parts(), 1);
  EXPECT_EQ(p.whole(), 5);
  EXPECT_THAT(p, ElementsAre(5));
  EXPECT_EQ(p[0], 5);
}

TEST(Partition, Rectangle) {
  Partition p = Partition::Rectangle(3, 5);
  EXPECT_EQ(p.parts(), 3);
  EXPECT_EQ(p.whole(), 15);
  EXPECT_THAT(p, ElementsAre(5, 5, 5));
  EXPECT_EQ(p[0], 5);
  EXPECT_EQ(p[1], 5);
  EXPECT_EQ(p[2], 5);
}

TEST(Partition, Conjugate) {
  EXPECT_EQ(Partition::Trivial().conjugate(), Partition::Trivial());
  EXPECT_EQ(Partition::Rectangle(3, 5).conjugate(), Partition::Rectangle(5, 3));
  EXPECT_EQ((Partition{3, 1, 1}.conjugate()), (Partition{3, 1, 1}));
  EXPECT_EQ((Partition{3, 1}.conjugate()), (Partition{2, 1, 1}));
  EXPECT_EQ((Partition{3, 3, 1}.conjugate()), (Partition{3, 2, 2}));
  EXPECT_EQ((Partition{3, 2, 2}.conjugate()), (Partition{3, 3, 1}));
}

TEST(Partition, Rank) {
  EXPECT_EQ(Rank(Partition::Trivial()), 0);
  EXPECT_EQ(Rank(Partition{5, 1, 1}), 1);
  EXPECT_EQ(Rank(Partition{2, 1, 1}), 1);
  EXPECT_EQ(Rank(Partition{2, 2, 1}), 2);
  EXPECT_EQ(Rank(Partition{8, 2, 2}), 2);
  EXPECT_EQ(Rank(Partition{8, 4, 3, 2}), 3);
  EXPECT_EQ(Rank(Partition::Rectangle(5, 5)), 5);
  EXPECT_EQ(Rank(Partition::Rectangle(5, 6)), 5);
  EXPECT_EQ(Rank(Partition::Rectangle(6, 5)), 5);
}

TEST(Factorial, Correct) {
  EXPECT_EQ(Factorial(0), 1);
  EXPECT_EQ(Factorial(1), 1);
  EXPECT_EQ(Factorial(2), 2);
  EXPECT_EQ(Factorial(3), 6);
  EXPECT_EQ(Factorial(10), 3628800);
}

TEST(Partition, Factorial) {
  EXPECT_EQ(Factorial(Partition::Trivial()), 1);
  EXPECT_EQ(Factorial(Partition{5, 2, 1}), 240);
  EXPECT_EQ(Factorial(Partition{4, 4}), 576);
  EXPECT_EQ(Factorial(Partition{2, 2, 2, 2}), 16);
}

TEST(Partition, CycleTypeCount) {
  EXPECT_EQ(CycleTypeCount(Partition::Trivial()), 1);
  EXPECT_EQ(CycleTypeCount(Partition{5}), 24);
  EXPECT_EQ(CycleTypeCount(Partition{4, 1}), 30);
  EXPECT_EQ(CycleTypeCount(Partition{3, 2}), 20);
  EXPECT_EQ(CycleTypeCount(Partition{3, 1, 1}), 20);
  EXPECT_EQ(CycleTypeCount(Partition{2, 2, 1}), 15);
  EXPECT_EQ(CycleTypeCount(Partition{2, 1, 1, 1}), 10);
  EXPECT_EQ(CycleTypeCount(Partition{1, 1, 1, 1, 1}), 1);
}

}  // namespace chalk


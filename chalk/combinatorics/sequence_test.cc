#include "chalk/combinatorics/sequence.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {

using ::testing::ElementsAre;
using ::testing::IsEmpty;

TEST(GapsBetween, Empty) {
  std::vector<int> v;
  EXPECT_THAT(GapsBetween(v, 0), IsEmpty());
}

TEST(GapsBetween, NotPresent) {
  EXPECT_THAT(GapsBetween(std::vector{1}, 0), ElementsAre(1));
  EXPECT_THAT(GapsBetween(std::vector{0, 1, 2, 3}, 10), ElementsAre(4));
}

TEST(GapsBetween, OnlyMatches) {
  EXPECT_THAT(GapsBetween(std::vector{1}, 1), ElementsAre(0, 0));
  EXPECT_THAT(GapsBetween(std::vector{1, 1, 1}, 1), ElementsAre(0, 0, 0, 0));
}

TEST(GapsBetween, General) {
  EXPECT_THAT(GapsBetween(std::vector{1, 2, 1, 3, 4, 1, 5}, 1),
              ElementsAre(0, 1, 2, 1));
  EXPECT_THAT(GapsBetween(std::vector{0, 2, 1, 3, 4, 1, 5}, 1),
              ElementsAre(2, 2, 1));
}

}  // namespace chalk

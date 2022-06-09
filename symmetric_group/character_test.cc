#include "symmetric_group/character.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {

using S = SymmetricGroupCharacter;

TEST(SymmetricGroupCharacter, One) {
  EXPECT_EQ(S::Irreducible({1}), S::KroneckerDelta({1}));
}

TEST(SymmetricGroupCharacter, Two) {
  auto c11 = S::KroneckerDelta({1, 1});
  auto c2  = S::KroneckerDelta({2});
  EXPECT_EQ(S::Irreducible({2}), c11 + c2);
  EXPECT_EQ(S::Irreducible({1, 1}), c11 - c2);
}

TEST(SymmetricGroupCharacter, Three) {
  auto c111 = S::KroneckerDelta({1, 1, 1});
  auto c21  = S::KroneckerDelta({2, 1});
  auto c3   = S::KroneckerDelta({3});
  EXPECT_EQ(S::Irreducible({3}), c111 + c21 + c3);
  EXPECT_EQ(S::Irreducible({2, 1}), 2 * c111 - c3);
  EXPECT_EQ(S::Irreducible({1, 1, 1}), c111 - c21 + c3);
}

TEST(SymmetricGroupCharacter, Four) {
  auto c1111 = S::KroneckerDelta({1, 1, 1, 1});
  auto c211  = S::KroneckerDelta({2, 1, 1});
  auto c22   = S::KroneckerDelta({2, 2});
  auto c31   = S::KroneckerDelta({3, 1});
  auto c4    = S::KroneckerDelta({4});
  EXPECT_EQ(S::Irreducible({4}), c1111 + c211 + c22 + c31 + c4);
  EXPECT_EQ(S::Irreducible({3,1}), 3 * c1111 + c211 - c22 - c4);
  EXPECT_EQ(S::Irreducible({2,2}), 2 * c1111 + 2 * c22 - c31);
  EXPECT_EQ(S::Irreducible({2,1,1}), 3 * c1111 - c211 - c22 + c4);
  EXPECT_EQ(S::Irreducible({1, 1, 1, 1}), c1111 - c211 + c22 + c31 - c4);
}

TEST(SymmetricGroupCharacter, Four) {
  auto c1111 = S::KroneckerDelta({1, 1, 1, 1});
  auto c211  = S::KroneckerDelta({2, 1, 1});
  auto c22   = S::KroneckerDelta({2, 2});
  auto c31   = S::KroneckerDelta({3, 1});
  auto c4    = S::KroneckerDelta({4});
  EXPECT_EQ(S::Irreducible({4}), c1111 + c211 + c22 + c31 + c4);
  EXPECT_EQ(S::Irreducible({3,1}), 3 * c1111 + c211 - c22 - c4);
  EXPECT_EQ(S::Irreducible({2,2}), 2 * c1111 + 2 * c22 - c31);
  EXPECT_EQ(S::Irreducible({2,1,1}), 3 * c1111 - c211 - c22 + c4);
  EXPECT_EQ(S::Irreducible({1, 1, 1, 1}), c1111 - c211 + c22 + c31 - c4);
}

}  // namespace chalk

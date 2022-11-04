#include "chalk/base/lazy.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk::base {
namespace {

TEST(Lazy, Lazy) {
  bool b = false;
  Lazy l([&] {
    b = true;
    return 3;
  });
  EXPECT_FALSE(b);
  EXPECT_EQ(l, 3);
  EXPECT_TRUE(b);
}

}  // namespace
}  // namespace chalk::base

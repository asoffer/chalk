#include "integer.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace chalk {
namespace {

Integer Factorial(size_t n) {
  Integer result = 1;
  for (size_t i = 2; i <= n; ++i) { result *= i; }
  return result;
}

std::string ToString(Integer const& n) {
  std::stringstream ss;
  ss << n;
  return ss.str();
}

TEST(Integer, Construction) {
  Integer zero;
  EXPECT_TRUE(zero == 0);
  EXPECT_FALSE(zero != 0);
  EXPECT_FALSE(zero == 1);
  EXPECT_TRUE(zero != 1);
  EXPECT_FALSE(zero == -1);
  EXPECT_TRUE(zero != -1);

  Integer one = 1;
  EXPECT_TRUE(one == 1);
  EXPECT_FALSE(one != 1);
  EXPECT_FALSE(one == 0);
  EXPECT_TRUE(one != 0);
  EXPECT_FALSE(one == -1);
  EXPECT_TRUE(one != -1);

  Integer minus_one = -1;
  EXPECT_TRUE(minus_one == -1);
  EXPECT_FALSE(minus_one != -1);
  EXPECT_FALSE(minus_one == 0);
  EXPECT_TRUE(minus_one != 0);
  EXPECT_FALSE(minus_one == 1);
  EXPECT_TRUE(minus_one != 1);
}

TEST(Integer, Overflow) {
  Integer n = (uint64_t{1} << 63);
  n *= 8;

  Integer m = (uint64_t{1} << 60);
  m *= 64;
  EXPECT_EQ(n, m);

  n = std::numeric_limits<uint64_t>::max();
  n += 1;

  m = 1;
  m += std::numeric_limits<uint64_t>::max();
  EXPECT_EQ(n, m);
}

TEST(Integer, Factorial) {
  EXPECT_EQ(ToString(Factorial(0)), "0x1");
  EXPECT_EQ(ToString(Factorial(1)), "0x1");
  EXPECT_EQ(ToString(Factorial(2)), "0x2");
  EXPECT_EQ(ToString(Factorial(3)), "0x6");
  EXPECT_EQ(ToString(Factorial(4)), "0x18");
  EXPECT_EQ(ToString(Factorial(5)), "0x78");
  EXPECT_EQ(ToString(Factorial(6)), "0x2d0");
  EXPECT_EQ(ToString(Factorial(7)), "0x13b0");
  EXPECT_EQ(ToString(Factorial(8)), "0x9d80");
  EXPECT_EQ(ToString(Factorial(9)), "0x58980");
  EXPECT_EQ(ToString(Factorial(10)), "0x375f00");
  EXPECT_EQ(ToString(Factorial(11)), "0x2611500");
  EXPECT_EQ(ToString(Factorial(12)), "0x1c8cfc00");
  EXPECT_EQ(ToString(Factorial(13)), "0x17328cc00");
  EXPECT_EQ(ToString(Factorial(14)), "0x144c3b2800");
  EXPECT_EQ(ToString(Factorial(15)), "0x13077775800");
  EXPECT_EQ(ToString(Factorial(16)), "0x130777758000");
  EXPECT_EQ(ToString(Factorial(17)), "0x1437eeecd8000");
  EXPECT_EQ(ToString(Factorial(18)), "0x16beecca730000");
  EXPECT_EQ(ToString(Factorial(19)), "0x1b02b9306890000");
  EXPECT_EQ(ToString(Factorial(20)), "0x21c3677c82b40000");
  EXPECT_EQ(ToString(Factorial(21)), "0x2c5077d36b8c40000");
  EXPECT_EQ(ToString(Factorial(22)), "0x3ceea4c2b3e0d80000");
  EXPECT_EQ(ToString(Factorial(23)), "0x57970cd7e2933680000");
  EXPECT_EQ(ToString(Factorial(23)), "0x57970cd7e2933680000");
  EXPECT_EQ(ToString(Factorial(24)), "0x83629343d3dcd1c00000");
  EXPECT_EQ(ToString(Factorial(25)), "0xcd4a0619fb0907bc00000");
  EXPECT_EQ(ToString(Factorial(26)), "0x14d9849ea37eeac91800000");
  EXPECT_EQ(ToString(Factorial(27)), "0x232f0fcbb3e62c3358800000");
  EXPECT_EQ(ToString(Factorial(28)), "0x3d925ba47ad2cd59dae000000");
  EXPECT_EQ(ToString(Factorial(29)), "0x6f99461a1e9e1432dcb6000000");
  EXPECT_EQ(ToString(Factorial(30)), "0xd13f6370f96865df5dd54000000");
  EXPECT_EQ(ToString(Factorial(31)), "0x1956ad0aae33a4560c5cd2c000000");
  EXPECT_EQ(ToString(Factorial(32)), "0x32ad5a155c6748ac18b9a580000000");
  EXPECT_EQ(ToString(Factorial(33)), "0x688589cc0e9505e2f2fee5580000000");
  EXPECT_EQ(ToString(Factorial(34)), "0xde1bc4d19efcac82445da75b00000000");
  EXPECT_EQ(ToString(Factorial(35)), "0x1e5dcbe8a8bc8b95cf58cde17100000000");
  EXPECT_EQ(ToString(Factorial(36)), "0x44530acb7ba83a111287cf3b3e400000000");
  EXPECT_EQ(ToString(Factorial(37)), "0x9e0008f68df506477ada0f38fff400000000");
  EXPECT_EQ(ToString(Factorial(38)),
            "0x1774015499125eee9c3c5e4275fe3800000000");
  EXPECT_EQ(ToString(Factorial(39)),
            "0x392ac33e351cc7659cd325c1ff9ba8800000000");
  EXPECT_EQ(ToString(Factorial(40)),
            "0x8eeae81b84c7f27e080fde64ff05254000000000");
  EXPECT_EQ(ToString(Factorial(41)),
            "0x16e39f2c684405d62f4a8a9e2cd7d2f74000000000");
  EXPECT_EQ(ToString(Factorial(42)),
            "0x3c1581d491b28f523c23abdf35b689c908000000000");
  EXPECT_EQ(ToString(Factorial(43)),
            "0xa179cceb478fe12d019fdde7e05a924c458000000000");
  EXPECT_EQ(ToString(Factorial(44)),
            "0x1bc0ef38704cbab3bc477a23da8f91251bf20000000000");
  EXPECT_EQ(ToString(Factorial(45)),
            "0x4e0ea0cebbd7cd1981890784d6b3c8385e98a0000000000");
  EXPECT_EQ(ToString(Factorial(46)),
            "0xe06a0e525c0c6da95469f59de944dfa20ff6cc0000000000");
  EXPECT_EQ(ToString(Factorial(47)),
            "0x293378a11ee64822167f7417fdd3a50ec0ee4f740000000000");
  EXPECT_EQ(ToString(Factorial(48)),
            "0x7b9a69e35cb2d866437e5c47f97aef2c42caee5c00000000000");
  EXPECT_EQ(ToString(Factorial(49)),
            "0x17a88e4484be3b6b92eb2fa9c6c087c778c8d79f9c00000000000");
  EXPECT_EQ(ToString(Factorial(50)),
            "0x49eebc961ed279b02b1ef4f28d19a84f5973a1d2c7800000000000");
  EXPECT_EQ(ToString(Factorial(51)),
            "0xeba8f91e823ee3e18972acc521c1c87ced2093cfdbe800000000000");
  EXPECT_EQ(ToString(Factorial(52)),
            "0x2fde529a3274c649cfeb4b180adb5cb9602a9e0638ab2000000000000");
  EXPECT_EQ(ToString(Factorial(53)),
            "0x9e90719ec722d0d480bb68bfa3f6a3260e8d2b749bb6da000000000000");
  EXPECT_EQ(ToString(Factorial(54)),
            "0x217277f77e01580cd32788186c96066a0711c72a98d891fc000000000000");
  EXPECT_EQ(ToString(Factorial(55)),
            "0x72f97c62c1249eac15d7e3d3f543b60c784d1ca26d6875d24000000000000");
  EXPECT_EQ(
      ToString(Factorial(56)),
      "0x192693359a4002b5a4c739d65da6cfd2ba50de4387eed9c5fe0000000000000");
  EXPECT_EQ(
      ToString(Factorial(57)),
      "0x59996c6ef58409a71b05be0bada2445eb7c017d09442e7d158e0000000000000");
  EXPECT_EQ(
      ToString(Factorial(58)),
      "0x144cc291239fea2fdc1f4d0ea556c37d75a18565419728856e22c0000000000000");
  EXPECT_EQ(
      ToString(Factorial(59)),
      "0x4adb0d77335daf907bb36c2601aff0dea1c39be561dd656c0620240000000000000");
  EXPECT_EQ(ToString(Factorial(60)),
            "0x118b5727f009f525dcfe0d58e8653c742de9d889c2efe3c5516f887000000000"
            "00000");
  EXPECT_EQ(ToString(Factorial(61)),
            "0x42e33c484325f6a05a8892e2f601f67aef0b898d373294604679382b00000000"
            "000000");
  EXPECT_EQ(ToString(Factorial(62)),
            "0x10330899804331bad5ed1392f79479b1c5e4cb50335e3fef51115b9a6a000000"
            "00000000");
  EXPECT_EQ(ToString(Factorial(63)),
            "0x3fc8f1dc690893cfaa557d12aed89f2bfb34e08bca431bbe4f3458b001600000"
            "000000000");
}


TEST(Integer, Multiplication) {
  EXPECT_EQ(Integer(2) * Integer(5), 10);
  EXPECT_EQ(Integer(2) * Integer(5), Integer(10));
}

}  // namespace
}  // namespace chalk

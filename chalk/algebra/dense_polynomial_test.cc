#include "chalk/algebra/dense_polynomial.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {
namespace {

using ::testing::ElementsAre;

TEST(DensePolynomial, FromCoefficients) {
  using polynomial_type = DensePolynomial<int64_t>;

  {
    auto p = polynomial_type::FromCoefficients({});
    EXPECT_EQ(p.degree(), -1);
    EXPECT_THAT(p.coefficients(), ElementsAre());
  }

  {
    auto p = polynomial_type::FromCoefficients({5});
    EXPECT_EQ(p.degree(), 0);
    EXPECT_THAT(p.coefficients(), ElementsAre(5));
  }

  {
    auto p = polynomial_type::FromCoefficients({0});
    EXPECT_EQ(p.degree(), -1);
    EXPECT_THAT(p.coefficients(), ElementsAre());
  }

  {
    auto p = polynomial_type::FromCoefficients({1, 2, 3});
    EXPECT_EQ(p.degree(), 2);
    EXPECT_THAT(p.coefficients(), ElementsAre(1, 2, 3));
  }

  {
    auto p = polynomial_type::FromCoefficients({1, 2, 3, 0, 0});
    EXPECT_EQ(p.degree(), 2);
    EXPECT_THAT(p.coefficients(), ElementsAre(1, 2, 3));
  }
}

TEST(DensePolynomial, FromCoefficientsWithIterator) {
  using polynomial_type = DensePolynomial<int64_t>;

  {
    std::vector<int64_t> v;
    auto p = polynomial_type::FromCoefficients(v.begin(), v.end());
    EXPECT_EQ(p.degree(), -1);
    EXPECT_THAT(p.coefficients(), ElementsAre());
  }

  {
    std::vector<int64_t> v{5};
    auto p = polynomial_type::FromCoefficients(v.begin(), v.end());
    EXPECT_EQ(p.degree(), 0);
    EXPECT_THAT(p.coefficients(), ElementsAre(5));
  }

  {
    std::vector<int64_t> v{0};
    auto p = polynomial_type::FromCoefficients(v.begin(), v.end());
    EXPECT_EQ(p.degree(), -1);
    EXPECT_THAT(p.coefficients(), ElementsAre());
  }

  {
    std::vector<int64_t> v{1, 2, 3};
    auto p = polynomial_type::FromCoefficients(v.begin(), v.end());
    EXPECT_EQ(p.degree(), 2);
    EXPECT_THAT(p.coefficients(), ElementsAre(1, 2, 3));
  }

  {
    std::vector<int64_t> v{1, 2, 3, 0, 0};
    auto p = polynomial_type::FromCoefficients(v.begin(), v.end());
    EXPECT_EQ(p.degree(), 2);
    EXPECT_THAT(p.coefficients(), ElementsAre(1, 2, 3));
  }
}

TEST(DensePolynomial, Equality) {
  using polynomial_type = DensePolynomial<int64_t>;

  auto a = polynomial_type::FromCoefficients({});
  auto b = polynomial_type::FromCoefficients({5});
  auto c = polynomial_type::FromCoefficients({1, 2, 3});

  EXPECT_TRUE(a == a);
  EXPECT_FALSE(a == b);
  EXPECT_FALSE(a == c);

  EXPECT_FALSE(b == a);
  EXPECT_TRUE(b == b);
  EXPECT_FALSE(b == c);

  EXPECT_FALSE(c == a);
  EXPECT_FALSE(c == b);
  EXPECT_TRUE(c == c);

  EXPECT_FALSE(a != a);
  EXPECT_TRUE(a != b);
  EXPECT_TRUE(a != c);

  EXPECT_TRUE(b != a);
  EXPECT_FALSE(b != b);
  EXPECT_TRUE(b != c);

  EXPECT_TRUE(c != a);
  EXPECT_TRUE(c != b);
  EXPECT_FALSE(c != c);
}

TEST(DensePolynomial, Construction) {
  using polynomial_type = DensePolynomial<int64_t>;
  polynomial_type p;
  EXPECT_EQ(p, polynomial_type::FromCoefficients({}));
  EXPECT_EQ((Unit<'+', polynomial_type>()),
            polynomial_type::FromCoefficients({}));
  EXPECT_EQ((Unit<'*', polynomial_type>()),
            polynomial_type::FromCoefficients({1}));
  EXPECT_EQ(polynomial_type::Zero(), polynomial_type::FromCoefficients({}));
  EXPECT_EQ(polynomial_type::One(), polynomial_type::FromCoefficients({1}));
  EXPECT_EQ((Unit<'*', polynomial_type>()),
            polynomial_type::FromCoefficients({1}));
  EXPECT_EQ(polynomial_type(3), polynomial_type::FromCoefficients({3}));
}

TEST(DensePolynomial, Degree) {
  using polynomial_type = DensePolynomial<int64_t>;

  auto a = polynomial_type::FromCoefficients({});
  EXPECT_EQ(a.degree(), -1);
  EXPECT_THAT(a.coefficients(), ElementsAre());
  EXPECT_EQ(a.coefficient(0), 0);
  EXPECT_EQ(a.coefficient(1), 0);
  EXPECT_EQ(a.coefficient(2), 0);
  EXPECT_EQ(a.coefficient(3), 0);
  EXPECT_EQ(a.coefficient(4), 0);

  auto b = polynomial_type::FromCoefficients({5});
  EXPECT_EQ(b.degree(), 0);
  EXPECT_THAT(b.coefficients(), ElementsAre(5));
  EXPECT_EQ(b.coefficient(0), 5);
  EXPECT_EQ(b.coefficient(1), 0);
  EXPECT_EQ(b.coefficient(2), 0);
  EXPECT_EQ(b.coefficient(3), 0);
  EXPECT_EQ(b.coefficient(4), 0);

  auto c = polynomial_type::FromCoefficients({1, 2, 3});
  EXPECT_EQ(c.degree(), 2);
  EXPECT_THAT(c.coefficients(), ElementsAre(1, 2, 3));
  EXPECT_EQ(c.coefficient(0), 1);
  EXPECT_EQ(c.coefficient(1), 2);
  EXPECT_EQ(c.coefficient(2), 3);
  EXPECT_EQ(c.coefficient(3), 0);
  EXPECT_EQ(c.coefficient(4), 0);
}

TEST(DensePolynomial, Addition) {
  using polynomial_type = DensePolynomial<int64_t>;

  auto a = polynomial_type::FromCoefficients({});
  auto b = polynomial_type::FromCoefficients({5});
  auto c = polynomial_type::FromCoefficients({1, 2, 3, 4});
  auto d = polynomial_type::FromCoefficients({1, 2, -3, -4});
  auto e = polynomial_type::FromCoefficients({-5});

  EXPECT_THAT((a + a).coefficients(), ElementsAre());
  EXPECT_THAT((a + b).coefficients(), ElementsAre(5));
  EXPECT_THAT((a + c).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT((a + d).coefficients(), ElementsAre(1, 2, -3, -4));
  EXPECT_THAT((a + e).coefficients(), ElementsAre(-5));

  EXPECT_THAT((b + a).coefficients(), ElementsAre(5));
  EXPECT_THAT((b + b).coefficients(), ElementsAre(10));
  EXPECT_THAT((b + c).coefficients(), ElementsAre(6, 2, 3, 4));
  EXPECT_THAT((b + d).coefficients(), ElementsAre(6, 2, -3, -4));
  EXPECT_THAT((b + e).coefficients(), ElementsAre());

  EXPECT_THAT((c + a).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT((c + b).coefficients(), ElementsAre(6, 2, 3, 4));
  EXPECT_THAT((c + c).coefficients(), ElementsAre(2, 4, 6, 8));
  EXPECT_THAT((c + d).coefficients(), ElementsAre(2, 4));
  EXPECT_THAT((c + e).coefficients(), ElementsAre(-4, 2, 3, 4));

  EXPECT_THAT((d + a).coefficients(), ElementsAre(1, 2, -3, -4));
  EXPECT_THAT((d + b).coefficients(), ElementsAre(6, 2, -3, -4));
  EXPECT_THAT((d + c).coefficients(), ElementsAre(2, 4));
  EXPECT_THAT((d + d).coefficients(), ElementsAre(2, 4, -6, -8));
  EXPECT_THAT((d + e).coefficients(), ElementsAre(-4, 2, -3, -4));

  EXPECT_THAT((e + a).coefficients(), ElementsAre(-5));
  EXPECT_THAT((e + b).coefficients(), ElementsAre());
  EXPECT_THAT((e + c).coefficients(), ElementsAre(-4, 2, 3, 4));
  EXPECT_THAT((e + d).coefficients(), ElementsAre(-4, 2, -3, -4));
  EXPECT_THAT((e + e).coefficients(), ElementsAre(-10));

  auto add = [](polynomial_type lhs, auto const& rhs) { return lhs += rhs; };

  EXPECT_THAT(add(a, a).coefficients(), ElementsAre());
  EXPECT_THAT(add(a, b).coefficients(), ElementsAre(5));
  EXPECT_THAT(add(a, c).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT(add(a, d).coefficients(), ElementsAre(1, 2, -3, -4));
  EXPECT_THAT(add(a, e).coefficients(), ElementsAre(-5));

  EXPECT_THAT(add(b, a).coefficients(), ElementsAre(5));
  EXPECT_THAT(add(b, b).coefficients(), ElementsAre(10));
  EXPECT_THAT(add(b, c).coefficients(), ElementsAre(6, 2, 3, 4));
  EXPECT_THAT(add(b, d).coefficients(), ElementsAre(6, 2, -3, -4));
  EXPECT_THAT(add(b, e).coefficients(), ElementsAre());

  EXPECT_THAT(add(c, a).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT(add(c, b).coefficients(), ElementsAre(6, 2, 3, 4));
  EXPECT_THAT(add(c, c).coefficients(), ElementsAre(2, 4, 6, 8));
  EXPECT_THAT(add(c, d).coefficients(), ElementsAre(2, 4));
  EXPECT_THAT(add(c, e).coefficients(), ElementsAre(-4, 2, 3, 4));

  EXPECT_THAT(add(d, a).coefficients(), ElementsAre(1, 2, -3, -4));
  EXPECT_THAT(add(d, b).coefficients(), ElementsAre(6, 2, -3, -4));
  EXPECT_THAT(add(d, c).coefficients(), ElementsAre(2, 4));
  EXPECT_THAT(add(d, d).coefficients(), ElementsAre(2, 4, -6, -8));
  EXPECT_THAT(add(d, e).coefficients(), ElementsAre(-4, 2, -3, -4));

  EXPECT_THAT(add(e, a).coefficients(), ElementsAre(-5));
  EXPECT_THAT(add(e, b).coefficients(), ElementsAre());
  EXPECT_THAT(add(e, c).coefficients(), ElementsAre(-4, 2, 3, 4));
  EXPECT_THAT(add(e, d).coefficients(), ElementsAre(-4, 2, -3, -4));
  EXPECT_THAT(add(e, e).coefficients(), ElementsAre(-10));

  EXPECT_THAT((c + 0).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT((0 + c).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT((c + 3).coefficients(), ElementsAre(4, 2, 3, 4));
  EXPECT_THAT((3 + c).coefficients(), ElementsAre(4, 2, 3, 4));
  EXPECT_THAT(add(c, 0).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT(add(0, c).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT(add(c, 3).coefficients(), ElementsAre(4, 2, 3, 4));
  EXPECT_THAT(add(3, c).coefficients(), ElementsAre(4, 2, 3, 4));
}

TEST(DensePolynomial, Subtraction) {
  using polynomial_type = DensePolynomial<int64_t>;

  auto a = polynomial_type::FromCoefficients({});
  auto b = polynomial_type::FromCoefficients({5});
  auto c = polynomial_type::FromCoefficients({1, 2, 3, 4});
  auto d = polynomial_type::FromCoefficients({-1, -2, 3, 4});
  auto e = polynomial_type::FromCoefficients({-5});

  EXPECT_THAT((a - a).coefficients(), ElementsAre());
  EXPECT_THAT((a - b).coefficients(), ElementsAre(-5));
  EXPECT_THAT((a - c).coefficients(), ElementsAre(-1, -2, -3, -4));
  EXPECT_THAT((a - d).coefficients(), ElementsAre(1, 2, -3, -4));
  EXPECT_THAT((a - e).coefficients(), ElementsAre(5));

  EXPECT_THAT((b - a).coefficients(), ElementsAre(5));
  EXPECT_THAT((b - b).coefficients(), ElementsAre());
  EXPECT_THAT((b - c).coefficients(), ElementsAre(4, -2, -3, -4));
  EXPECT_THAT((b - d).coefficients(), ElementsAre(6, 2, -3, -4));
  EXPECT_THAT((b - e).coefficients(), ElementsAre(10));

  EXPECT_THAT((c - a).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT((c - b).coefficients(), ElementsAre(-4, 2, 3, 4));
  EXPECT_THAT((c - c).coefficients(), ElementsAre());
  EXPECT_THAT((c - d).coefficients(), ElementsAre(2, 4));
  EXPECT_THAT((c - e).coefficients(), ElementsAre(6, 2, 3, 4));

  EXPECT_THAT((d - a).coefficients(), ElementsAre(-1, -2, 3, 4));
  EXPECT_THAT((d - b).coefficients(), ElementsAre(-6, -2, 3, 4));
  EXPECT_THAT((d - c).coefficients(), ElementsAre(-2, -4));
  EXPECT_THAT((d - d).coefficients(), ElementsAre());
  EXPECT_THAT((d - e).coefficients(), ElementsAre(4, -2, 3, 4));

  EXPECT_THAT((e - a).coefficients(), ElementsAre(-5));
  EXPECT_THAT((e - b).coefficients(), ElementsAre(-10));
  EXPECT_THAT((e - c).coefficients(), ElementsAre(-6, -2, -3, -4));
  EXPECT_THAT((e - d).coefficients(), ElementsAre(-4, 2, -3, -4));
  EXPECT_THAT((e - e).coefficients(), ElementsAre());

  auto sub = [](polynomial_type lhs, auto const& rhs) { return lhs -= rhs; };

  EXPECT_THAT(sub(a, a).coefficients(), ElementsAre());
  EXPECT_THAT(sub(a, b).coefficients(), ElementsAre(-5));
  EXPECT_THAT(sub(a, c).coefficients(), ElementsAre(-1, -2, -3, -4));
  EXPECT_THAT(sub(a, d).coefficients(), ElementsAre(1, 2, -3, -4));
  EXPECT_THAT(sub(a, e).coefficients(), ElementsAre(5));

  EXPECT_THAT(sub(b, a).coefficients(), ElementsAre(5));
  EXPECT_THAT(sub(b, b).coefficients(), ElementsAre());
  EXPECT_THAT(sub(b, c).coefficients(), ElementsAre(4, -2, -3, -4));
  EXPECT_THAT(sub(b, d).coefficients(), ElementsAre(6, 2, -3, -4));
  EXPECT_THAT(sub(b, e).coefficients(), ElementsAre(10));

  EXPECT_THAT(sub(c, a).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT(sub(c, b).coefficients(), ElementsAre(-4, 2, 3, 4));
  EXPECT_THAT(sub(c, c).coefficients(), ElementsAre());
  EXPECT_THAT(sub(c, d).coefficients(), ElementsAre(2, 4));
  EXPECT_THAT(sub(c, e).coefficients(), ElementsAre(6, 2, 3, 4));

  EXPECT_THAT(sub(d, a).coefficients(), ElementsAre(-1, -2, 3, 4));
  EXPECT_THAT(sub(d, b).coefficients(), ElementsAre(-6, -2, 3, 4));
  EXPECT_THAT(sub(d, c).coefficients(), ElementsAre(-2, -4));
  EXPECT_THAT(sub(d, d).coefficients(), ElementsAre());
  EXPECT_THAT(sub(d, e).coefficients(), ElementsAre(4, -2, 3, 4));

  EXPECT_THAT(sub(e, a).coefficients(), ElementsAre(-5));
  EXPECT_THAT(sub(e, b).coefficients(), ElementsAre(-10));
  EXPECT_THAT(sub(e, c).coefficients(), ElementsAre(-6, -2, -3, -4));
  EXPECT_THAT(sub(e, d).coefficients(), ElementsAre(-4, 2, -3, -4));
  EXPECT_THAT(sub(e, e).coefficients(), ElementsAre());

  EXPECT_THAT((c - 0).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT((c - 3).coefficients(), ElementsAre(-2, 2, 3, 4));
  EXPECT_THAT(sub(c, 0).coefficients(), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT(sub(c, 3).coefficients(), ElementsAre(-2, 2, 3, 4));
}

TEST(DensePolynomial, Multiplication) {
  using polynomial_type = DensePolynomial<int64_t>;

  auto a = polynomial_type::FromCoefficients({});
  auto b = polynomial_type::FromCoefficients({5});
  auto c = polynomial_type::FromCoefficients({1, 2, 1});
  auto d = polynomial_type::FromCoefficients({-1, 1});

  EXPECT_THAT((a * a).coefficients(), ElementsAre());
  EXPECT_THAT((a * b).coefficients(), ElementsAre());
  EXPECT_THAT((a * c).coefficients(), ElementsAre());
  EXPECT_THAT((a * d).coefficients(), ElementsAre());

  EXPECT_THAT((b * a).coefficients(), ElementsAre());
  EXPECT_THAT((b * b).coefficients(), ElementsAre(25));
  EXPECT_THAT((b * c).coefficients(), ElementsAre(5, 10, 5));
  EXPECT_THAT((b * d).coefficients(), ElementsAre(-5, 5));

  EXPECT_THAT((c * a).coefficients(), ElementsAre());
  EXPECT_THAT((c * b).coefficients(), ElementsAre(5, 10, 5));
  EXPECT_THAT((c * c).coefficients(), ElementsAre(1, 4, 6, 4, 1));
  EXPECT_THAT((c * d).coefficients(), ElementsAre(-1, -1, 1, 1));

  EXPECT_THAT((d * a).coefficients(), ElementsAre());
  EXPECT_THAT((d * b).coefficients(), ElementsAre(-5, 5));
  EXPECT_THAT((d * c).coefficients(), ElementsAre(-1, -1, 1, 1));
  EXPECT_THAT((d * d).coefficients(), ElementsAre(1, -2, 1));

  auto mul = [](polynomial_type lhs, auto const& rhs) { return lhs *= rhs; };

  EXPECT_THAT(mul(a, a).coefficients(), ElementsAre());
  EXPECT_THAT(mul(a, b).coefficients(), ElementsAre());
  EXPECT_THAT(mul(a, c).coefficients(), ElementsAre());
  EXPECT_THAT(mul(a, d).coefficients(), ElementsAre());

  EXPECT_THAT(mul(b, a).coefficients(), ElementsAre());
  EXPECT_THAT(mul(b, b).coefficients(), ElementsAre(25));
  EXPECT_THAT(mul(b, c).coefficients(), ElementsAre(5, 10, 5));
  EXPECT_THAT(mul(b, d).coefficients(), ElementsAre(-5, 5));

  EXPECT_THAT(mul(c, a).coefficients(), ElementsAre());
  EXPECT_THAT(mul(c, b).coefficients(), ElementsAre(5, 10, 5));
  EXPECT_THAT(mul(c, c).coefficients(), ElementsAre(1, 4, 6, 4, 1));
  EXPECT_THAT(mul(c, d).coefficients(), ElementsAre(-1, -1, 1, 1));

  EXPECT_THAT(mul(d, a).coefficients(), ElementsAre());
  EXPECT_THAT(mul(d, b).coefficients(), ElementsAre(-5, 5));
  EXPECT_THAT(mul(d, c).coefficients(), ElementsAre(-1, -1, 1, 1));
  EXPECT_THAT(mul(d, d).coefficients(), ElementsAre(1, -2, 1));

  EXPECT_EQ(c * 0, 0);
  EXPECT_EQ(0 * c, 0);
  EXPECT_EQ(mul(c, 0), 0);

  EXPECT_EQ(c * 1, c);
  EXPECT_EQ(1 * c, c);
  EXPECT_EQ(mul(c, 1), c);

  EXPECT_THAT((c * 3).coefficients(), ElementsAre(3, 6, 3));
  EXPECT_THAT((3 * c).coefficients(), ElementsAre(3, 6, 3));
  EXPECT_THAT(mul(c, 3).coefficients(), ElementsAre(3, 6, 3));
}

struct ZMod2 : Algebraic {
  using chalk_properties = void(Ring);

  ZMod2(uint8_t n = 0) : value_(n & 1u) {}

  ZMod2& operator-=(ZMod2 n) {
    value_ = (value_ + n.value_) & 1u;
    return *this;
  }

  ZMod2& operator+=(ZMod2 n) {
    value_ = (value_ + n.value_) & 1u;
    return *this;
  }

  friend bool operator==(ZMod2 l, ZMod2 r) = default;

 private:
  uint8_t value_;
};

TEST(DensePolynomial, SubringConversion) {
  using polynomial_type = DensePolynomial<double>;

  auto p = polynomial_type::FromCoefficients({1.5, 2.5});
  p *= 3;
  EXPECT_THAT(p.coefficients(), ElementsAre(4.5, 7.5));

  auto q = DensePolynomial<int>::FromCoefficients({1, 1});
  p *= q;
  EXPECT_THAT(p.coefficients(), ElementsAre(4.5, 12.0, 7.5));
  p = q;
  EXPECT_THAT(p.coefficients(), ElementsAre(1.0, 1.0));
}

TEST(DensePolynomial, ImplicitQuotient) {
  DensePolynomial<ZMod2> p = DensePolynomial<int>::FromCoefficients({1, 2, 4});
  EXPECT_EQ(p, 1);
}

}  // namespace
}  // namespace chalk

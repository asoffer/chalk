#include "chalk/algebra/polynomial.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {
namespace {

TEST(Polynomial, FromTerms) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();
  {
    auto p = polynomial_type::FromTerms({});
    EXPECT_EQ(p.term_count(), 0);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 0);
    EXPECT_EQ(p.coefficient(x), 0);
    EXPECT_EQ(p.coefficient(y), 0);
    EXPECT_EQ(p.coefficient(z), 0);
  }

  {
    auto p = polynomial_type::FromTerms({{x, 0}});
    EXPECT_EQ(p.term_count(), 0);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 0);
    EXPECT_EQ(p.coefficient(x), 0);
    EXPECT_EQ(p.coefficient(y), 0);
    EXPECT_EQ(p.coefficient(z), 0);
  }

  {
    auto p = polynomial_type::FromTerms({{x, 1}});
    EXPECT_EQ(p.term_count(), 1);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 0);
    EXPECT_EQ(p.coefficient(x), 1);
    EXPECT_EQ(p.coefficient(y), 0);
    EXPECT_EQ(p.coefficient(z), 0);
  }

  {
    auto p = polynomial_type::FromTerms({{x, 1}, {x, -1}});
    EXPECT_EQ(p.term_count(), 0);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 0);
    EXPECT_EQ(p.coefficient(x), 0);
    EXPECT_EQ(p.coefficient(y), 0);
    EXPECT_EQ(p.coefficient(z), 0);
  }

  {
    auto p = polynomial_type::FromTerms({{x, 1}, {y, -1}});
    EXPECT_EQ(p.term_count(), 2);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 0);
    EXPECT_EQ(p.coefficient(x), 1);
    EXPECT_EQ(p.coefficient(y), -1);
    EXPECT_EQ(p.coefficient(z), 0);
  }

  {
    auto p = polynomial_type::FromTerms({{Unit<'*'>(), 5},
                                         {monomial_type{} * x * x, 1},
                                         {monomial_type{} * x * y, -1}});
    EXPECT_EQ(p.term_count(), 3);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 5);
    EXPECT_EQ(p.coefficient(monomial_type{} * x * x), 1);
    EXPECT_EQ(p.coefficient(monomial_type{} * x * y), -1);
  }
}

TEST(Polynomial, FromTermsWithIterators) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();
  {
    std::vector<std::pair<monomial_type, int64_t>> v{};
    auto p = polynomial_type::FromTerms(v.begin(), v.end());
    EXPECT_EQ(p.term_count(), 0);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 0);
    EXPECT_EQ(p.coefficient(x), 0);
    EXPECT_EQ(p.coefficient(y), 0);
    EXPECT_EQ(p.coefficient(z), 0);
  }

  {
    std::vector<std::pair<monomial_type, int64_t>> v{{x, 0}};
    auto p = polynomial_type::FromTerms(v.begin(), v.end());
    EXPECT_EQ(p.term_count(), 0);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 0);
    EXPECT_EQ(p.coefficient(x), 0);
    EXPECT_EQ(p.coefficient(y), 0);
    EXPECT_EQ(p.coefficient(z), 0);
  }

  {
    std::vector<std::pair<monomial_type, int64_t>> v{{x, 1}};
    auto p = polynomial_type::FromTerms(v.begin(), v.end());
    EXPECT_EQ(p.term_count(), 1);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 0);
    EXPECT_EQ(p.coefficient(x), 1);
    EXPECT_EQ(p.coefficient(y), 0);
    EXPECT_EQ(p.coefficient(z), 0);
  }

  {
    std::vector<std::pair<monomial_type, int64_t>> v{{x, 1}, {x, -1}};
    auto p = polynomial_type::FromTerms(v.begin(), v.end());
    EXPECT_EQ(p.term_count(), 0);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 0);
    EXPECT_EQ(p.coefficient(x), 0);
    EXPECT_EQ(p.coefficient(y), 0);
    EXPECT_EQ(p.coefficient(z), 0);
  }

  {
    std::vector<std::pair<monomial_type, int64_t>> v{{x, 1}, {y, -1}};
    auto p = polynomial_type::FromTerms(v.begin(), v.end());
    EXPECT_EQ(p.term_count(), 2);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 0);
    EXPECT_EQ(p.coefficient(x), 1);
    EXPECT_EQ(p.coefficient(y), -1);
    EXPECT_EQ(p.coefficient(z), 0);
  }

  {
    std::vector<std::pair<monomial_type, int64_t>> v{
        {Unit<'*'>(), 5},
        {monomial_type{} * x * x, 1},
        {monomial_type{} * x * y, -1}};
    auto p = polynomial_type::FromTerms(v.begin(), v.end());
    EXPECT_EQ(p.term_count(), 3);
    EXPECT_EQ(p.coefficient(Unit<'*'>()), 5);
    EXPECT_EQ(p.coefficient(monomial_type{} * x * x), 1);
    EXPECT_EQ(p.coefficient(monomial_type{} * x * y), -1);
  }
}

TEST(Polynomial, Equality) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();

  auto a = polynomial_type::FromTerms({});
  auto b = polynomial_type::FromTerms({{x, 3}});
  auto c = polynomial_type::FromTerms({{y, -3}});
  auto d = polynomial_type::FromTerms(
      {{monomial_type{} * x * y, 1}, {x, 5}, {Unit<'*'>(), 1}});
  auto e = polynomial_type::FromTerms({{y, 0}, {x, 3}});

  EXPECT_TRUE(a == a);
  EXPECT_FALSE(a == b);
  EXPECT_FALSE(a == c);
  EXPECT_FALSE(a == d);
  EXPECT_FALSE(a == e);

  EXPECT_FALSE(b == a);
  EXPECT_TRUE(b == b);
  EXPECT_FALSE(b == c);
  EXPECT_FALSE(b == d);
  EXPECT_TRUE(b == e);

  EXPECT_FALSE(c == a);
  EXPECT_FALSE(c == b);
  EXPECT_TRUE(c == c);
  EXPECT_FALSE(c == d);
  EXPECT_FALSE(c == e);

  EXPECT_FALSE(d == a);
  EXPECT_FALSE(d == b);
  EXPECT_FALSE(d == c);
  EXPECT_TRUE(d == d);
  EXPECT_FALSE(d == e);

  EXPECT_FALSE(e == a);
  EXPECT_TRUE(e == b);
  EXPECT_FALSE(e == c);
  EXPECT_FALSE(e == d);
  EXPECT_TRUE(e == e);

  EXPECT_FALSE(a != a);
  EXPECT_TRUE(a != b);
  EXPECT_TRUE(a != c);
  EXPECT_TRUE(a != d);
  EXPECT_TRUE(a != e);

  EXPECT_TRUE(b != a);
  EXPECT_FALSE(b != b);
  EXPECT_TRUE(b != c);
  EXPECT_TRUE(b != d);
  EXPECT_FALSE(b != e);

  EXPECT_TRUE(c != a);
  EXPECT_TRUE(c != b);
  EXPECT_FALSE(c != c);
  EXPECT_TRUE(c != d);
  EXPECT_TRUE(c != e);

  EXPECT_TRUE(d != a);
  EXPECT_TRUE(d != b);
  EXPECT_TRUE(d != c);
  EXPECT_FALSE(d != d);
  EXPECT_TRUE(d != e);

  EXPECT_TRUE(e != a);
  EXPECT_FALSE(e != b);
  EXPECT_TRUE(e != c);
  EXPECT_TRUE(e != d);
  EXPECT_FALSE(e != e);
}

TEST(Polynomial, EqualityAgainstCoefficientRing) {
  using polynomial_type = Polynomial<int64_t, 3>;
  auto [x, y, z]        = polynomial_type::Variables();

  auto a = polynomial_type::FromTerms({});
  auto b = polynomial_type::FromTerms({{x, 3}});
  auto c = polynomial_type::FromTerms({{Unit<'*'>(), 3}});

  EXPECT_TRUE(a == 0);
  EXPECT_TRUE(0 == a);
  EXPECT_FALSE(a == 1);
  EXPECT_FALSE(1 == a);
  EXPECT_FALSE(a == 3);
  EXPECT_FALSE(3 == a);

  EXPECT_FALSE(b == 0);
  EXPECT_FALSE(0 == b);
  EXPECT_FALSE(b == 1);
  EXPECT_FALSE(1 == b);
  EXPECT_FALSE(b == 3);
  EXPECT_FALSE(3 == b);

  EXPECT_FALSE(c == 0);
  EXPECT_FALSE(0 == c);
  EXPECT_FALSE(c == 1);
  EXPECT_FALSE(1 == c);
  EXPECT_TRUE(c == 3);
  EXPECT_TRUE(3 == c);

  EXPECT_FALSE(a != 0);
  EXPECT_FALSE(0 != a);
  EXPECT_TRUE(a != 1);
  EXPECT_TRUE(1 != a);
  EXPECT_TRUE(a != 3);
  EXPECT_TRUE(3 != a);

  EXPECT_TRUE(b != 0);
  EXPECT_TRUE(0 != b);
  EXPECT_TRUE(b != 1);
  EXPECT_TRUE(1 != b);
  EXPECT_TRUE(b != 3);
  EXPECT_TRUE(3 != b);

  EXPECT_TRUE(c != 0);
  EXPECT_TRUE(0 != c);
  EXPECT_TRUE(c != 1);
  EXPECT_TRUE(1 != c);
  EXPECT_FALSE(c != 3);
  EXPECT_FALSE(3 != c);
}

TEST(Polynomial, EqualityAgainstMonomial) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();

  auto a = polynomial_type::FromTerms({});
  auto b = polynomial_type::FromTerms({{x, 1}});
  auto c = polynomial_type::FromTerms({{Unit<'*'>(), 3}});
  auto d = polynomial_type::FromTerms({{Unit<'*'>(), 1}});

  monomial_type unit = Unit<'*'>();
  monomial_type m    = x;

  EXPECT_FALSE(a == unit);
  EXPECT_FALSE(unit == a);
  EXPECT_FALSE(a == m);
  EXPECT_FALSE(m == a);
  EXPECT_FALSE(a == x);
  EXPECT_FALSE(x == a);

  EXPECT_FALSE(b == unit);
  EXPECT_FALSE(unit == b);
  EXPECT_TRUE(b == m);
  EXPECT_TRUE(m == b);
  EXPECT_TRUE(b == x);
  EXPECT_TRUE(x == b);

  EXPECT_FALSE(c == unit);
  EXPECT_FALSE(unit == c);
  EXPECT_FALSE(c == m);
  EXPECT_FALSE(m == c);
  EXPECT_FALSE(c == x);
  EXPECT_FALSE(x == c);

  EXPECT_TRUE(d == unit);
  EXPECT_TRUE(unit == d);
  EXPECT_FALSE(d == m);
  EXPECT_FALSE(m == d);
  EXPECT_FALSE(d == x);
  EXPECT_FALSE(x == d);

  EXPECT_TRUE(a != unit);
  EXPECT_TRUE(unit != a);
  EXPECT_TRUE(a != m);
  EXPECT_TRUE(m != a);
  EXPECT_TRUE(a != x);
  EXPECT_TRUE(x != a);

  EXPECT_TRUE(b != unit);
  EXPECT_TRUE(unit != b);
  EXPECT_FALSE(b != m);
  EXPECT_FALSE(m != b);
  EXPECT_FALSE(b != x);
  EXPECT_FALSE(x != b);

  EXPECT_TRUE(c != unit);
  EXPECT_TRUE(unit != c);
  EXPECT_TRUE(c != m);
  EXPECT_TRUE(m != c);
  EXPECT_TRUE(c != x);
  EXPECT_TRUE(x != c);

  EXPECT_FALSE(d != unit);
  EXPECT_FALSE(unit != d);
  EXPECT_TRUE(d != m);
  EXPECT_TRUE(m != d);
  EXPECT_TRUE(d != x);
  EXPECT_TRUE(x != d);
}

TEST(Polynomial, Construction) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();

  polynomial_type p;
  EXPECT_EQ(p, polynomial_type::FromTerms({}));
  EXPECT_EQ(polynomial_type::Zero(), polynomial_type::FromTerms({}));
  EXPECT_EQ(polynomial_type::One(),
            polynomial_type::FromTerms({{Unit<'*'>(), 1}}));

  EXPECT_EQ(polynomial_type(x), polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(polynomial_type(y), polynomial_type::FromTerms({{y, 1}}));

  EXPECT_EQ(polynomial_type(monomial_type{} * x),
            polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(polynomial_type(monomial_type{} * y),
            polynomial_type::FromTerms({{y, 1}}));
  EXPECT_EQ(polynomial_type(monomial_type{} * x * y),
            polynomial_type::FromTerms({{monomial_type{} * x * y, 1}}));

  EXPECT_EQ(polynomial_type(1), polynomial_type::FromTerms({{Unit<'*'>(), 1}}));
  EXPECT_EQ(polynomial_type(5), polynomial_type::FromTerms({{Unit<'*'>(), 5}}));
  EXPECT_EQ(polynomial_type(0), polynomial_type::FromTerms({}));
}

TEST(Polynomial, Addition) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();

  polynomial_type a = 0;
  polynomial_type b = 3;
  polynomial_type c = x;
  polynomial_type d = Unit<'*', polynomial_type>() * x * x - y + 1;

  EXPECT_EQ(a + 0, a);
  EXPECT_EQ(0 + a, a);
  EXPECT_EQ(a + 1, polynomial_type::FromTerms({{Unit<'*'>(), 1}}));
  EXPECT_EQ(1 + a, polynomial_type::FromTerms({{Unit<'*'>(), 1}}));
  EXPECT_EQ(a + 3, polynomial_type::FromTerms({{Unit<'*'>(), 3}}));
  EXPECT_EQ(3 + a, polynomial_type::FromTerms({{Unit<'*'>(), 3}}));
  EXPECT_EQ(a + x, polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(x + a, polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(a + monomial_type{x}, polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(monomial_type{x} + a, polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(a + a, 0);
  EXPECT_EQ(a + b, polynomial_type::FromTerms({{Unit<'*'>(), 3}}));
  EXPECT_EQ(b + a, polynomial_type::FromTerms({{Unit<'*'>(), 3}}));
  EXPECT_EQ(a + c, polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(c + a, polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(a + d,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 1}}));
  EXPECT_EQ(d + a,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 1}}));

  EXPECT_EQ(b + 0, b);
  EXPECT_EQ(0 + b, b);
  EXPECT_EQ(b + 1, polynomial_type::FromTerms({{Unit<'*'>(), 4}}));
  EXPECT_EQ(1 + b, polynomial_type::FromTerms({{Unit<'*'>(), 4}}));
  EXPECT_EQ(b + 3, polynomial_type::FromTerms({{Unit<'*'>(), 6}}));
  EXPECT_EQ(3 + b, polynomial_type::FromTerms({{Unit<'*'>(), 6}}));
  EXPECT_EQ(b + x, polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(x + b, polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(b + monomial_type{x},
            polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(monomial_type{x} + b,
            polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(b + b, 6);
  EXPECT_EQ(b + c, polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(c + b, polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(b + d,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 4}}));
  EXPECT_EQ(d + b,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 4}}));

  EXPECT_EQ(c + 0, c);
  EXPECT_EQ(0 + c, c);
  EXPECT_EQ(c + 1, polynomial_type::FromTerms({{Unit<'*'>(), 1}, {x, 1}}));
  EXPECT_EQ(1 + c, polynomial_type::FromTerms({{Unit<'*'>(), 1}, {x, 1}}));
  EXPECT_EQ(c + 3, polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(3 + c, polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(c + x, polynomial_type::FromTerms({{x, 2}}));
  EXPECT_EQ(x + c, polynomial_type::FromTerms({{x, 2}}));
  EXPECT_EQ(c + monomial_type{x}, polynomial_type::FromTerms({{x, 2}}));
  EXPECT_EQ(monomial_type{x} + c, polynomial_type::FromTerms({{x, 2}}));
  EXPECT_EQ(c + c, polynomial_type::FromTerms({{x, 2}}));
  EXPECT_EQ(
      c + d,
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 1}, {x, 1}}));
  EXPECT_EQ(
      d + c,
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 1}, {x, 1}}));

  EXPECT_EQ(d + 0, d);
  EXPECT_EQ(0 + d, d);
  EXPECT_EQ(d + 1,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 2}}));
  EXPECT_EQ(1 + d,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 2}}));
  EXPECT_EQ(d + 3,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 4}}));
  EXPECT_EQ(3 + d,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 4}}));
  EXPECT_EQ(
      d + x,
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {x, 1}, {y, -1}, {Unit<'*'>(), 1}}));
  EXPECT_EQ(
      x + d,
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {x, 1}, {y, -1}, {Unit<'*'>(), 1}}));
  EXPECT_EQ(
      d + monomial_type{x},
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {x, 1}, {y, -1}, {Unit<'*'>(), 1}}));
  EXPECT_EQ(
      monomial_type{x} + d,
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {x, 1}, {y, -1}, {Unit<'*'>(), 1}}));
  EXPECT_EQ(d + d,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 2}, {y, -2}, {Unit<'*'>(), 2}}));

  auto add = [](polynomial_type p, auto const& rhs) { return p += rhs; };

  EXPECT_EQ(add(a, 0), a);
  EXPECT_EQ(add(a, 1), polynomial_type::FromTerms({{Unit<'*'>(), 1}}));
  EXPECT_EQ(add(a, 3), polynomial_type::FromTerms({{Unit<'*'>(), 3}}));
  EXPECT_EQ(add(a, x), polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(add(a, monomial_type{x}), polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(add(a, a), 0);
  EXPECT_EQ(add(a, b), polynomial_type::FromTerms({{Unit<'*'>(), 3}}));
  EXPECT_EQ(add(a, c), polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(add(a, d),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 1}}));

  EXPECT_EQ(add(b, 0), b);
  EXPECT_EQ(add(b, 1), polynomial_type::FromTerms({{Unit<'*'>(), 4}}));
  EXPECT_EQ(add(b, 3), polynomial_type::FromTerms({{Unit<'*'>(), 6}}));
  EXPECT_EQ(add(b, x), polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(add(b, monomial_type{x}),
            polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(add(b, b), 6);
  EXPECT_EQ(add(b, c), polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(add(b, d),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 4}}));

  EXPECT_EQ(add(c, 0), c);
  EXPECT_EQ(add(c, 1), polynomial_type::FromTerms({{Unit<'*'>(), 1}, {x, 1}}));
  EXPECT_EQ(add(c, 3), polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, 1}}));
  EXPECT_EQ(add(c, x), polynomial_type::FromTerms({{x, 2}}));
  EXPECT_EQ(add(c, monomial_type{x}), polynomial_type::FromTerms({{x, 2}}));
  EXPECT_EQ(add(c, c), polynomial_type::FromTerms({{x, 2}}));
  EXPECT_EQ(
      add(c, d),
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 1}, {x, 1}}));

  EXPECT_EQ(add(d, 0), d);
  EXPECT_EQ(add(d, 1),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 2}}));
  EXPECT_EQ(add(d, 3),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 4}}));
  EXPECT_EQ(
      add(d, x),
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {x, 1}, {y, -1}, {Unit<'*'>(), 1}}));
  EXPECT_EQ(
      add(d, monomial_type{x}),
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {x, 1}, {y, -1}, {Unit<'*'>(), 1}}));
  EXPECT_EQ(add(d, d),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 2}, {y, -2}, {Unit<'*'>(), 2}}));
}

TEST(Polynomial, Subtraction) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();

  polynomial_type a = 0;
  polynomial_type b = 3;
  polynomial_type c = x;
  polynomial_type d = Unit<'*', polynomial_type>() * x * x - y + 1;

  EXPECT_EQ(a - 0, 0);
  EXPECT_EQ(0 - a, 0);
  EXPECT_EQ(a - 1, polynomial_type::FromTerms({{Unit<'*'>(), -1}}));
  EXPECT_EQ(1 - a, polynomial_type::FromTerms({{Unit<'*'>(), 1}}));
  EXPECT_EQ(a - 3, polynomial_type::FromTerms({{Unit<'*'>(), -3}}));
  EXPECT_EQ(3 - a, polynomial_type::FromTerms({{Unit<'*'>(), 3}}));
  EXPECT_EQ(a - x, polynomial_type::FromTerms({{x, -1}}));
  EXPECT_EQ(x - a, polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(a - monomial_type{x}, polynomial_type::FromTerms({{x, -1}}));
  EXPECT_EQ(monomial_type{x} - a, polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(a - a, 0);
  EXPECT_EQ(a - b, polynomial_type::FromTerms({{Unit<'*'>(), -3}}));
  EXPECT_EQ(b - a, polynomial_type::FromTerms({{Unit<'*'>(), 3}}));
  EXPECT_EQ(a - c, polynomial_type::FromTerms({{x, -1}}));
  EXPECT_EQ(c - a, polynomial_type::FromTerms({{x, 1}}));
  EXPECT_EQ(a - d,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, -1}, {y, 1}, {Unit<'*'>(), -1}}));
  EXPECT_EQ(d - a,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 1}}));

  EXPECT_EQ(b - 0, 3);
  EXPECT_EQ(0 - b, -3);
  EXPECT_EQ(b - 1, polynomial_type::FromTerms({{Unit<'*'>(), 2}}));
  EXPECT_EQ(1 - b, polynomial_type::FromTerms({{Unit<'*'>(), -2}}));
  EXPECT_EQ(b - 3, 0);
  EXPECT_EQ(3 - b, 0);
  EXPECT_EQ(b - x, polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, -1}}));
  EXPECT_EQ(x - b, polynomial_type::FromTerms({{Unit<'*'>(), -3}, {x, 1}}));
  EXPECT_EQ(b - monomial_type{x},
            polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, -1}}));
  EXPECT_EQ(monomial_type{x} - b,
            polynomial_type::FromTerms({{Unit<'*'>(), -3}, {x, 1}}));
  EXPECT_EQ(b - b, 0);
  EXPECT_EQ(b - c, polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, -1}}));
  EXPECT_EQ(c - b, polynomial_type::FromTerms({{Unit<'*'>(), -3}, {x, 1}}));
  EXPECT_EQ(b - d,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, -1}, {y, 1}, {Unit<'*'>(), 2}}));
  EXPECT_EQ(d - b,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), -2}}));

  EXPECT_EQ(c - 0, c);
  EXPECT_EQ(0 - c, polynomial_type::FromTerms({{x, -1}}));
  EXPECT_EQ(c - 1, polynomial_type::FromTerms({{Unit<'*'>(), -1}, {x, 1}}));
  EXPECT_EQ(1 - c, polynomial_type::FromTerms({{Unit<'*'>(), 1}, {x, -1}}));
  EXPECT_EQ(c - 3, polynomial_type::FromTerms({{Unit<'*'>(), -3}, {x, 1}}));
  EXPECT_EQ(3 - c, polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, -1}}));
  EXPECT_EQ(c - x, 0);
  EXPECT_EQ(x - c, 0);
  EXPECT_EQ(c - monomial_type{x}, 0);
  EXPECT_EQ(monomial_type{x} - c, 0);
  EXPECT_EQ(c - c, 0);
  EXPECT_EQ(
      c - d,
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, -1}, {y, 1}, {Unit<'*'>(), -1}, {x, 1}}));
  EXPECT_EQ(
      d - c,
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), 1}, {x, -1}}));

  EXPECT_EQ(d - 0, d);
  EXPECT_EQ(0 - d,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, -1}, {y, 1}, {Unit<'*'>(), -1}}));
  EXPECT_EQ(d - 1, polynomial_type::FromTerms(
                       {{monomial_type{} * x * x, 1}, {y, -1}}));
  EXPECT_EQ(1 - d, polynomial_type::FromTerms(
                       {{monomial_type{} * x * x, -1}, {y, 1}}));
  EXPECT_EQ(d - 3,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), -2}}));
  EXPECT_EQ(3 - d,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, -1}, {y, 1}, {Unit<'*'>(), 2}}));
  EXPECT_EQ(
      d - x,
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {x, -1}, {y, -1}, {Unit<'*'>(), 1}}));
  EXPECT_EQ(
      x - d,
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, -1}, {x, 1}, {y, 1}, {Unit<'*'>(), -1}}));
  EXPECT_EQ(
      d - monomial_type{x},
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {x, -1}, {y, -1}, {Unit<'*'>(), 1}}));
  EXPECT_EQ(
      monomial_type{x} - d,
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, -1}, {x, 1}, {y, 1}, {Unit<'*'>(), -1}}));
  EXPECT_EQ(d - d, 0);

  auto sub = [](polynomial_type p, auto const& rhs) { return p -= rhs; };

  EXPECT_EQ(sub(a, 0), 0);
  EXPECT_EQ(sub(a, 1), polynomial_type::FromTerms({{Unit<'*'>(), -1}}));
  EXPECT_EQ(sub(a, 3), polynomial_type::FromTerms({{Unit<'*'>(), -3}}));
  EXPECT_EQ(sub(a, x), polynomial_type::FromTerms({{x, -1}}));
  EXPECT_EQ(sub(a, monomial_type{x}), polynomial_type::FromTerms({{x, -1}}));
  EXPECT_EQ(sub(a, a), 0);
  EXPECT_EQ(sub(a, b), polynomial_type::FromTerms({{Unit<'*'>(), -3}}));
  EXPECT_EQ(sub(a, c), polynomial_type::FromTerms({{x, -1}}));
  EXPECT_EQ(sub(a, d),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, -1}, {y, 1}, {Unit<'*'>(), -1}}));
  EXPECT_EQ(sub(b, 0), 3);
  EXPECT_EQ(sub(b, 1), polynomial_type::FromTerms({{Unit<'*'>(), 2}}));
  EXPECT_EQ(sub(b, 3), 0);
  EXPECT_EQ(sub(b, x), polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, -1}}));
  EXPECT_EQ(sub(b, monomial_type{x}),
            polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, -1}}));
  EXPECT_EQ(sub(b, b), 0);
  EXPECT_EQ(sub(b, c), polynomial_type::FromTerms({{Unit<'*'>(), 3}, {x, -1}}));
  EXPECT_EQ(sub(b, d),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, -1}, {y, 1}, {Unit<'*'>(), 2}}));

  EXPECT_EQ(sub(c, 0), c);
  EXPECT_EQ(sub(c, 1), polynomial_type::FromTerms({{Unit<'*'>(), -1}, {x, 1}}));
  EXPECT_EQ(sub(c, 3), polynomial_type::FromTerms({{Unit<'*'>(), -3}, {x, 1}}));
  EXPECT_EQ(sub(c, x), 0);
  EXPECT_EQ(sub(c, monomial_type{x}), 0);
  EXPECT_EQ(sub(c, c), 0);
  EXPECT_EQ(
      sub(c, d),
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, -1}, {y, 1}, {Unit<'*'>(), -1}, {x, 1}}));

  EXPECT_EQ(sub(d, 0), d);
  EXPECT_EQ(sub(d, 1), polynomial_type::FromTerms(
                           {{monomial_type{} * x * x, 1}, {y, -1}}));
  EXPECT_EQ(sub(d, 3),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 1}, {y, -1}, {Unit<'*'>(), -2}}));
  EXPECT_EQ(
      sub(d, x),
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {x, -1}, {y, -1}, {Unit<'*'>(), 1}}));
  EXPECT_EQ(
      sub(d, monomial_type{x}),
      polynomial_type::FromTerms(
          {{monomial_type{} * x * x, 1}, {x, -1}, {y, -1}, {Unit<'*'>(), 1}}));
  EXPECT_EQ(sub(d, d), 0);
}

TEST(Polynomial, Multiplication) {
  using polynomial_type = Polynomial<int64_t, 3>;
  using monomial_type   = typename polynomial_type::monomial_type;
  auto [x, y, z]        = polynomial_type::Variables();

  polynomial_type a = 0;
  polynomial_type b = 3;
  polynomial_type c = x;
  polynomial_type d = Unit<'*', polynomial_type>() * x * x - y + 1;

  EXPECT_EQ(a * 0, 0);
  EXPECT_EQ(0 * a, 0);
  EXPECT_EQ(a * 1, 0);
  EXPECT_EQ(1 * a, 0);
  EXPECT_EQ(a * 3, 0);
  EXPECT_EQ(3 * a, 0);
  EXPECT_EQ(a * x, 0);
  EXPECT_EQ(x * a, 0);
  EXPECT_EQ(a * monomial_type{x}, 0);
  EXPECT_EQ(monomial_type{x} * a, 0);
  EXPECT_EQ(a * a, 0);
  EXPECT_EQ(a * b, 0);
  EXPECT_EQ(b * a, 0);
  EXPECT_EQ(a * c, 0);
  EXPECT_EQ(c * a, 0);
  EXPECT_EQ(a * d, 0);
  EXPECT_EQ(d * a, 0);

  EXPECT_EQ(b * 0, 0);
  EXPECT_EQ(0 * b, 0);
  EXPECT_EQ(b * 1, b);
  EXPECT_EQ(1 * b, b);
  EXPECT_EQ(b * 3, 9);
  EXPECT_EQ(3 * b, 9);
  EXPECT_EQ(b * x, polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(x * b, polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(b * monomial_type{x}, polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(monomial_type{x} * b, polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(b * b, 9);
  EXPECT_EQ(b * c, polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(c * b, polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(b * d,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 3}, {y, -3}, {Unit<'*'>(), 3}}));
  EXPECT_EQ(d * b,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 3}, {y, -3}, {Unit<'*'>(), 3}}));

  EXPECT_EQ(c * 0, 0);
  EXPECT_EQ(0 * c, 0);
  EXPECT_EQ(c * 1, c);
  EXPECT_EQ(1 * c, c);
  EXPECT_EQ(c * 3, polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(3 * c, polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(c * x, polynomial_type::FromTerms({{monomial_type{} * x * x, 1}}));
  EXPECT_EQ(x * c, polynomial_type::FromTerms({{monomial_type{} * x * x, 1}}));
  EXPECT_EQ(c * monomial_type{x},
            polynomial_type::FromTerms({{monomial_type{} * x * x, 1}}));
  EXPECT_EQ(monomial_type{x} * c,
            polynomial_type::FromTerms({{monomial_type{} * x * x, 1}}));
  EXPECT_EQ(c * c, polynomial_type::FromTerms({{monomial_type{} * x * x, 1}}));
  EXPECT_EQ(c * d, polynomial_type::FromTerms({{monomial_type{} * x * x * x, 1},
                                               {monomial_type{} * x * y, -1},
                                               {x, 1}}));
  EXPECT_EQ(d * c, polynomial_type::FromTerms({{monomial_type{} * x * x * x, 1},
                                               {monomial_type{} * x * y, -1},
                                               {x, 1}}));

  EXPECT_EQ(d * 0, 0);
  EXPECT_EQ(0 * d, 0);
  EXPECT_EQ(d * 1, d);
  EXPECT_EQ(1 * d, d);
  EXPECT_EQ(d * 3,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 3}, {y, -3}, {Unit<'*'>(), 3}}));
  EXPECT_EQ(3 * d,
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 3}, {y, -3}, {Unit<'*'>(), 3}}));
  EXPECT_EQ(d * x, polynomial_type::FromTerms({{monomial_type{x} * x * x, 1},
                                               {monomial_type{x} * y, -1},
                                               {x, 1}}));
  EXPECT_EQ(x * d, polynomial_type::FromTerms({{monomial_type{x} * x * x, 1},
                                               {monomial_type{x} * y, -1},
                                               {x, 1}}));
  EXPECT_EQ(
      d * monomial_type{x},
      polynomial_type::FromTerms(
          {{monomial_type{x} * x * x, 1}, {monomial_type{x} * y, -1}, {x, 1}}));
  EXPECT_EQ(
      monomial_type{x} * d,
      polynomial_type::FromTerms(
          {{monomial_type{x} * x * x, 1}, {monomial_type{x} * y, -1}, {x, 1}}));
  EXPECT_EQ(d * d,
            polynomial_type::FromTerms({{monomial_type{x} * x * x * x, 1},
                                        {monomial_type{x} * x * y, -2},
                                        {monomial_type{x} * x, 2},
                                        {monomial_type{y} * y, 1},
                                        {y, -2},
                                        {Unit<'*'>(), 1}}));

  auto mul = [](polynomial_type p, auto const& rhs) { return p *= rhs; };

  EXPECT_EQ(mul(a, 0), 0);
  EXPECT_EQ(mul(0, a), 0);
  EXPECT_EQ(mul(a, 1), 0);
  EXPECT_EQ(mul(1, a), 0);
  EXPECT_EQ(mul(a, 3), 0);
  EXPECT_EQ(mul(3, a), 0);
  EXPECT_EQ(mul(a, x), 0);
  EXPECT_EQ(mul(x, a), 0);
  EXPECT_EQ(mul(a, monomial_type{x}), 0);
  EXPECT_EQ(mul(monomial_type{x}, a), 0);
  EXPECT_EQ(mul(a, a), 0);
  EXPECT_EQ(mul(a, b), 0);
  EXPECT_EQ(mul(b, a), 0);
  EXPECT_EQ(mul(a, c), 0);
  EXPECT_EQ(mul(c, a), 0);
  EXPECT_EQ(mul(a, d), 0);
  EXPECT_EQ(mul(d, a), 0);

  EXPECT_EQ(mul(b, 0), 0);
  EXPECT_EQ(mul(0, b), 0);
  EXPECT_EQ(mul(b, 1), b);
  EXPECT_EQ(mul(1, b), b);
  EXPECT_EQ(mul(b, 3), 9);
  EXPECT_EQ(mul(3, b), 9);
  EXPECT_EQ(mul(b, x), polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(mul(x, b), polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(mul(b, monomial_type{x}), polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(mul(monomial_type{x}, b), polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(mul(b, b), 9);
  EXPECT_EQ(mul(b, c), polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(mul(c, b), polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(mul(b, d),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 3}, {y, -3}, {Unit<'*'>(), 3}}));
  EXPECT_EQ(mul(d, b),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 3}, {y, -3}, {Unit<'*'>(), 3}}));

  EXPECT_EQ(mul(c, 0), 0);
  EXPECT_EQ(mul(0, c), 0);
  EXPECT_EQ(mul(c, 1), c);
  EXPECT_EQ(mul(1, c), c);
  EXPECT_EQ(mul(c, 3), polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(mul(3, c), polynomial_type::FromTerms({{x, 3}}));
  EXPECT_EQ(mul(c, x),
            polynomial_type::FromTerms({{monomial_type{} * x * x, 1}}));
  EXPECT_EQ(mul(x, c),
            polynomial_type::FromTerms({{monomial_type{} * x * x, 1}}));
  EXPECT_EQ(mul(c, monomial_type{x}),
            polynomial_type::FromTerms({{monomial_type{} * x * x, 1}}));
  EXPECT_EQ(mul(monomial_type{x}, c),
            polynomial_type::FromTerms({{monomial_type{} * x * x, 1}}));
  EXPECT_EQ(mul(c, c),
            polynomial_type::FromTerms({{monomial_type{} * x * x, 1}}));
  EXPECT_EQ(mul(c, d),
            polynomial_type::FromTerms({{monomial_type{} * x * x * x, 1},
                                        {monomial_type{} * x * y, -1},
                                        {x, 1}}));
  EXPECT_EQ(mul(d, c),
            polynomial_type::FromTerms({{monomial_type{} * x * x * x, 1},
                                        {monomial_type{} * x * y, -1},
                                        {x, 1}}));

  EXPECT_EQ(mul(d, 0), 0);
  EXPECT_EQ(mul(0, d), 0);
  EXPECT_EQ(mul(d, 1), d);
  EXPECT_EQ(mul(1, d), d);
  EXPECT_EQ(mul(d, 3),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 3}, {y, -3}, {Unit<'*'>(), 3}}));
  EXPECT_EQ(mul(3, d),
            polynomial_type::FromTerms(
                {{monomial_type{} * x * x, 3}, {y, -3}, {Unit<'*'>(), 3}}));
  EXPECT_EQ(
      mul(d, x),
      polynomial_type::FromTerms(
          {{monomial_type{x} * x * x, 1}, {monomial_type{x} * y, -1}, {x, 1}}));
  EXPECT_EQ(
      mul(x, d),
      polynomial_type::FromTerms(
          {{monomial_type{x} * x * x, 1}, {monomial_type{x} * y, -1}, {x, 1}}));
  EXPECT_EQ(
      mul(d, monomial_type{x}),
      polynomial_type::FromTerms(
          {{monomial_type{x} * x * x, 1}, {monomial_type{x} * y, -1}, {x, 1}}));
  EXPECT_EQ(
      mul(monomial_type{x}, d),
      polynomial_type::FromTerms(
          {{monomial_type{x} * x * x, 1}, {monomial_type{x} * y, -1}, {x, 1}}));
  EXPECT_EQ(mul(d, d),
            polynomial_type::FromTerms({{monomial_type{x} * x * x * x, 1},
                                        {monomial_type{x} * x * y, -2},
                                        {monomial_type{x} * x, 2},
                                        {monomial_type{y} * y, 1},
                                        {y, -2},
                                        {Unit<'*'>(), 1}}));
}

// TODO: Tests for implicit conversions of rings that embed.

}  // namespace
}  // namespace chalk

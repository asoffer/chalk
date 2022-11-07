#ifndef CHALK_ALGEBRA_POLYNOMIAL_H
#define CHALK_ALGEBRA_POLYNOMIAL_H

#include <array>
#include <cstddef>
#include <ostream>

#include "absl/container/flat_hash_map.h"
#include "chalk/algebra/monomial.h"
#include "chalk/algebra/property.h"
#include "chalk/algebra/variable.h"
#include "chalk/base/lazy.h"
#include "chalk/base/type_traits.h"

namespace chalk {

template <Satisfies<Ring> CoefficientRing, size_t N, typename E = uint16_t>
struct Polynomial : Algebraic {
  using chalk_properties = void(
      Ring, std::conditional_t<Satisfies<CoefficientRing, Commutative<'*'>>,
                               Commutative<'*'>, Ring>);

  using monomial_type    = Monomial<N, E>;
  using coefficient_ring = CoefficientRing;

  // Constructs the zero polynomial.
  explicit Polynomial() = default;

  // Constructs the zero polynomial.
  static Polynomial Zero() { return Polynomial(); }

  // Constructs the multiplicative unit `1`.
  static Polynomial One() { return Polynomial(Unit<'*', coefficient_ring>()); }

  // Constructs a polynomial from any type implicitly convertible to the
  // `monomial_type`.
  template <std::convertible_to<monomial_type> M>
  Polynomial(M &&t) {
    coefficients_.try_emplace(std::forward<M>(t),
                              Unit<'*', coefficient_ring>());
  }

  // Constructs a polynomial of degree zero from any type implicitly convertible
  // to the `coefficient_ring`.
  template <std::convertible_to<coefficient_ring> R>
  Polynomial(R &&v) requires(not std::convertible_to<R, monomial_type>) {
    if (not IsUnit<'+'>(v)) {
      coefficients_.try_emplace(Unit<'*', monomial_type>(), std::forward<R>(v));
    }
  }

  // Constructs a polynomial equivalent to `p` embedded in `PolynomialRing`.
  template <Satisfies<Ring> R, size_t M>
  constexpr Polynomial(
      Polynomial<R, M, typename monomial_type::exponent_type> const
          &p) requires(M < monomial_type::variable_count and
                       std::convertible_to<R, coefficient_ring>) {
    for (auto const &[monomial, coef] : p.coefficients_) {
      coefficients_.try_emplace(monomial, coef);
    }
  }

  // Constructs a polynomial equivalent to `p` embedded in `PolynomialRing`.
  template <Satisfies<Ring> R, size_t M>
  constexpr Polynomial(
      Polynomial<R, M, typename monomial_type::exponent_type>
          &&p) requires(M < monomial_type::variable_count and
                        std::convertible_to<R, coefficient_ring>) {
    for (auto &[monomial, coef] : p.coefficients_) {
      coefficients_.try_emplace(monomial, std::move(coef));
    }
  }

  static Polynomial FromTerms(auto begin, auto end) {
    Polynomial result;
    for (auto iter = begin; iter != end; ++iter) {
      auto const &[m, r]  = *iter;
      auto [it, inserted] = result.coefficients_.try_emplace(m, r);
      if (not inserted) { it->second += r; }
    }
    erase_if(result.coefficients_,
             [](auto const &pair) { return IsUnit<'+'>(pair.second); });

    return result;
  }

  static Polynomial FromTerms(
      std::initializer_list<std::pair<monomial_type, coefficient_ring>>
          elements) {
    return FromTerms(elements.begin(), elements.end());
  }

  // Returns an array consisting of all variables usable in this Polynomial
  // ring.
  static auto Variables() { return monomial_type::Variables(); }

  size_t term_count() const { return coefficients_.size(); }

  coefficient_ring const &coefficient(monomial_type const &m) {
    auto iter = coefficients_.find(m);
    if (iter != coefficients_.end()) { return iter->second; }
    return Unit<'+', coefficient_ring>();
  }

  template <std::convertible_to<Polynomial> L,
            std::convertible_to<Polynomial> R>
  friend bool operator==(L const &lhs, R const &rhs) {
    if constexpr (not std::is_same_v<L, Polynomial>) {
      return rhs == lhs;
    } else {
      static_assert(std::is_same_v<L, Polynomial>);
      if constexpr (std::convertible_to<R, coefficient_ring>) {
        if (IsUnit<'+'>(rhs)) { return lhs.coefficients_.empty(); }
        if (lhs.coefficients_.size() != 1) { return false; }
        auto const &[m, c] = *lhs.coefficients_.begin();
        return IsUnit<'*'>(m) and c == rhs;
      } else if constexpr (std::convertible_to<R, monomial_type>) {
        if (lhs.coefficients_.size() != 1) { return false; }
        auto const &[m, c] = *lhs.coefficients_.begin();
        return m == rhs and IsUnit<'*'>(c);
      } else if constexpr (not std::is_same_v<R, Polynomial>) {
        return lhs == Polynomial(rhs);
      } else {
        if (lhs.coefficients_.size() != rhs.coefficients_.size()) {
          return false;
        }
        for (auto const &[m, coef] : lhs.coefficients_) {
          auto iter = rhs.coefficients_.find(m);
          if (iter == rhs.coefficients_.end()) { return false; }
          if (iter->second != coef) { return false; }
        }
        return true;
      }
    }
  }

  template <std::convertible_to<Polynomial> T>
  Polynomial &operator+=(T &&v) {
    using type = std::decay_t<T>;
    if constexpr (std::is_same_v<Polynomial, type>) {
      using R = base::forward_type<T, coefficient_ring>;
      for (auto &&[m, r] : v.coefficients_) {
        auto [iter, inserted] =
            coefficients_.try_emplace(m, std::forward<R>(r));
        auto &entry = iter->second;
        if (not inserted) { entry += std::forward<R>(r); }
        if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
      }
      return *this;
    } else if constexpr (std::convertible_to<T, monomial_type>) {
      auto const &one = Unit<'*', coefficient_ring>();
      auto [iter, inserted] =
          coefficients_.try_emplace(std::forward<T>(v), one);
      auto &entry = iter->second;
      if (not inserted) { entry += std::move(one); }
      if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
      return *this;
    } else if constexpr (std::convertible_to<T, coefficient_ring>) {
      auto constant_term    = Unit<'*', monomial_type>();
      auto [iter, inserted] = coefficients_.try_emplace(
          std::move(constant_term), std::forward<T>(v));
      auto &entry = iter->second;
      if (not inserted) { entry += std::forward<T>(v); }
      if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
      return *this;
    } else {
      return *this += Polynomial(v);
    }
  }

  template <std::convertible_to<Polynomial> T>
  Polynomial &operator-=(T &&v) {
    using type = std::decay_t<T>;
    if constexpr (std::is_same_v<Polynomial, type>) {
      using R = base::forward_type<T, coefficient_ring>;
      for (auto &&[m, r] : v.coefficients_) {
        auto [iter, inserted] = coefficients_.try_emplace(
            m, base::Lazy([r = &r] { return -std::forward<R>(*r); }));
        auto &entry = iter->second;
        if (not inserted) { entry -= std::forward<R>(r); }
        if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
      }
      return *this;
    } else if constexpr (std::convertible_to<T, monomial_type>) {
      auto const &one       = Unit<'*', coefficient_ring>();
      auto [iter, inserted] = coefficients_.try_emplace(
          std::forward<T>(v), base::Lazy([&] { return -one; }));
      auto &entry = iter->second;
      if (not inserted) { entry -= std::move(one); }
      if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
      return *this;
    } else if constexpr (std::convertible_to<T, coefficient_ring>) {
      auto constant_term    = Unit<'*', monomial_type>();
      auto [iter, inserted] = coefficients_.try_emplace(
          std::move(constant_term),
          base::Lazy([&] { return -std::forward<T>(v); }));
      auto &entry = iter->second;
      if (not inserted) { entry -= std::forward<T>(v); }
      if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
      return *this;
    } else {
      return *this -= Polynomial(v);
    }
  }

  template <std::convertible_to<Polynomial> T>
  Polynomial &operator*=(T &&v) {
    using type = std::decay_t<T>;
    if constexpr (std::is_same_v<Polynomial, type>) {
      auto coefs = std::exchange(coefficients_, {});
      Polynomial result;
      for (auto const &[lm, lr] : coefs) {
        for (auto const &[rm, rr] : v.coefficients_) {
          auto [iter, inserted] = coefficients_.try_emplace(
              lm * rm, base::Lazy([lr = &lr, rr = &rr] { return *lr * *rr; }));
          if (not inserted) { iter->second += lr * rr; }
        }
      }

      erase_if(coefficients_,
               [](auto const &pair) { return IsUnit<'+'>(pair.second); });
      return *this;
    } else if constexpr (std::convertible_to<T, monomial_type>) {
      auto coefs = std::exchange(coefficients_, {});
      while (not coefs.empty()) {
        auto handle = coefs.extract(coefs.begin());
        handle.key() *= v;
        coefficients_.insert(std::move(handle));
      }
      return *this;
    } else if constexpr (std::convertible_to<T, coefficient_ring>) {
      if (IsUnit<'+'>(v)) {
        coefficients_.clear();
      } else if (not IsUnit<'*'>(v)) {
        for (auto &[m, coef] : coefficients_) { coef *= v; }
      }
      return *this;
    } else {
      return *this *= Polynomial(v);
    }
  }

  friend std::ostream &operator<<(std::ostream &os, Polynomial const &p) {
    if (p.coefficients_.empty()) { return os << Unit<'+', coefficient_ring>(); }
    std::string_view separator = "";
    for (auto const &[m, r] : p.coefficients_) {
      os << std::exchange(separator, " + ");
      if (IsUnit<'*'>(m)) {
        os << r;
      } else {
        if (not IsUnit<'*'>(r)) { os << r; }
        os << m;
      }
    }
    return os;
  }

 private:
  template <Satisfies<Ring>, size_t, typename>
  friend struct Polynomial;

  absl::flat_hash_map<monomial_type, coefficient_ring> coefficients_;
};

}  // namespace chalk

#endif  // CHALK_ALGEBRA_POLYNOMIAL_H

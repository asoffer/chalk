#ifndef CHALK_ALGEBRA_POLYNOMIAL_H
#define CHALK_ALGEBRA_POLYNOMIAL_H

#include <array>
#include <cstddef>

#include "absl/container/flat_hash_map.h"
#include "chalk/algebra/monomial.h"
#include "chalk/algebra/property.h"
#include "chalk/algebra/variable.h"
#include "chalk/base/lazy.h"
#include "chalk/base/type_traits.h"

namespace chalk {

template <Satisfies<Ring> CoefficientRing, size_t N, typename E = uint16_t>
struct Polynomial : Algebraic {
  using chalk_properties = void(Ring);

  using monomial_type    = Monomial<N, E>;
  using coefficient_ring = CoefficientRing;

  // Constructs the zero polynomial
  explicit Polynomial() = default;

  static Polynomial Zero() { return Polynomial(); }
  static Polynomial One() { return Polynomial(Unit<'*', coefficient_ring>()); }

  template <std::convertible_to<monomial_type> M>
  Polynomial(M &&t) {
    coefficients_.try_emplace(std::forward<M>(t),
                              Unit<'*', coefficient_ring>());
  }

  template <std::convertible_to<coefficient_ring> R>
  Polynomial(R &&t) {
    coefficients_.try_emplace(Unit<'*', monomial_type>(), std::forward<R>(t));
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

  friend bool operator==(Polynomial const &lhs, Polynomial const &rhs) {
    if (lhs.coefficients_.size() != rhs.coefficients_.size()) { return false; }
    for (auto const &[m, r] : lhs.coefficients_) {
      auto iter = rhs.coefficients_.find(m);
      if (iter == rhs.coefficients_.end()) { return false; }
      if (iter->second != r) { return false; }
    }
    return true;
  }

  template <std::same_as<Polynomial> P>
  Polynomial &operator+=(P &&p) {
    using M = base::forward_type<P, monomial_type>;
    using R = base::forward_type<P, coefficient_ring>;

    for (auto &&[m, r] : p) {
      auto [iter, inserted] =
          coefficients_.try_emplace(std::forward<M>(m), std::forward<R>(r));
      if (not inserted) {
        auto &entry = iter->second;
        entry += std::forward<R>(r);
        if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
      }
    }
    return *this;
  }

  template <std::convertible_to<monomial_type> M>
  Polynomial &operator+=(M &&m) {
    auto one = Unit<'*', coefficient_ring>();
    auto [iter, inserted] =
        coefficients_.try_emplace(std::forward<M>(m), std::move(one));
    if (not inserted) {
      auto &entry = iter->second;
      entry += std::move(one);
      if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
    }
    return *this;
  }

  template <std::convertible_to<coefficient_ring> R>
  Polynomial &operator+=(R &&r) {
    auto constant_term = Unit<'*', monomial_type>();
    auto [iter, inserted] =
        coefficients_.try_emplace(std::move(constant_term), std::forward<R>(r));
    if (not inserted) {
      auto &entry = iter->second;
      entry += std::forward<R>(r);
      if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
    }
    return *this;
  }

  template <std::same_as<Polynomial> P>
  Polynomial &operator-=(P &&p) {
    using M = base::forward_type<P, monomial_type>;
    using R = base::forward_type<P, coefficient_ring>;

    for (auto &&[m, r] : p) {
      auto [iter, inserted] = coefficients_.try_emplace(
          std::forward<M>(m),
          base::Lazy([r = &r] { return -std::forward<R>(*r); }));
      if (not inserted) {
        auto &entry = iter->second;
        entry -= std::forward<R>(r);
        if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
      }
    }
    return *this;
  }

  template <std::convertible_to<monomial_type> M>
  Polynomial &operator-=(M &&m) {
    auto const &one       = Unit<'*', coefficient_ring>();
    auto [iter, inserted] = coefficients_.try_emplace(
        std::forward<M>(m), base::Lazy([&] { return -one; }));
    if (not inserted) {
      auto &entry = iter->second;
      entry -= one;
      if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
    }
    return *this;
  }

  template <std::convertible_to<coefficient_ring> R>
  Polynomial &operator-=(R &&r) {
    auto constant_term    = Unit<'*', monomial_type>();
    auto [iter, inserted] = coefficients_.try_emplace(
        std::move(constant_term),
        base::Lazy([&] { return -std::forward<R>(r); }));
    if (not inserted) {
      auto &entry = iter->second;
      entry -= std::forward<R>(r);
      if (IsUnit<'+'>(entry)) { coefficients_.erase(iter); }
    }
    return *this;
  }

  Polynomial &operator*=(Polynomial const &p) {
    Polynomial result;
    for (auto const &[lm, lr] : coefficients_) {
      for (auto const &[rm, rr] : p.coefficients_) {
        auto [iter, inserted] = result.try_emplace(
            lm * rm, base::Lazy([lr = &lr, rr = &rr] { return *lr * *rr; }));
        if (not inserted) { iter->second += lr * rr; }
      }
    }

    erase_if(result.coefficients_,
             [](auto const &pair) { return IsUnit<'+'>(pair.second); });
    return result;
  }

  template <std::convertible_to<monomial_type> M>
  Polynomial &operator*=(M const &m) {
    auto coefs = std::exchange(coefficients_, {});
    while (not coefs.empty()) {
      auto handle = coefs.extract(coefs.begin());
      handle.key() *= m;
      coefficients_.insert(std::move(handle));
    }
    return *this;
  }

  template <std::convertible_to<coefficient_ring> R>
  Polynomial &operator*=(R const &r) {
    if (IsUnit<'+'>(r)) {
      coefficients_.clear();
    } else if (not IsUnit<'*'>(r)) {
      for (auto &[m, coef] : coefficients_) { coef *= r; }
    }
    return *this;
  }

 private:
  absl::flat_hash_map<monomial_type, coefficient_ring> coefficients_;
};

}  // namespace chalk

#endif  // CHALK_ALGEBRA_POLYNOMIAL_H

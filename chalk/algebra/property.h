#ifndef CHALK_ALGEBRA_PROPERTY_H
#define CHALK_ALGEBRA_PROPERTY_H

#include <concepts>
#include <type_traits>
#include <utility>

#include "chalk/algebra/internal/property.h"

namespace chalk {

// Matches any structure whose properties (immediate and implied) match all of
// the `RequiredProperties`.
template <typename T, typename... RequiredProperties>
concept Satisfies =
    (internal_property::Contains<internal_property::PropertiesOf<T>,
                                 RequiredProperties>::value and
     ...);

// Indicates that the structure has a binary operator represented by `Op`.
template <char Op>
requires(internal_property::ValidOperator(Op)) struct HasBinary {
};

// Indicates that the structure has a commutative binary operator represented by
// `Op`, so that:
//   `x Op y == y Op x`
template <char Op>
requires(internal_property::ValidOperator(Op)) struct Commutative {
  using chalk_implies = void(HasBinary<Op>);
};

// Indicates that `Op1` distributes over `Op2`, so that:
//   `x Op1 (y Op2 z) == (x Op1 y) Op2 (x Op1 z)`
template <char Op1, char Op2>
requires(internal_property::ValidOperator(Op1) and
         internal_property::ValidOperator(Op2)) struct Distributes {
};

// Indicates that T acts on the structure via `*'.
template <typename T>
struct ScalableBy {};

template <char Op>
requires(internal_property::ValidOperator(Op)) struct Invertible {
  // TODO
};

// Indicates that the structure has an identity element for the binary operator
// represented by `Op`.
template <char Op>
requires(internal_property::ValidOperator(Op)) struct HasIdentity {
  using chalk_implies = void(HasBinary<Op>);
};

// Indicates that the structure is a monoid over the binary operator represented
// by `Op`.
template <char Op>
requires(internal_property::ValidOperator(Op)) struct Monoid {
  using chalk_implies = void(HasBinary<Op>, HasIdentity<Op>);
};

template <char Op>
requires(internal_property::ValidOperator(Op)) struct Group {
  using chalk_implies = void(Monoid<Op>, Invertible<Op>);
};

template <char Op>
requires(internal_property::ValidOperator(Op)) struct AbelianGroup;

template <typename T>
struct ModuleOver {
  using chalk_implies = void(AbelianGroup<'+'>, ScalableBy<T>);
};

// TODO: Rather than having to specify ModuleOver<X> for lots of specific X, we
// should be able to pick one and check implicit conversions.
template <char Op>
requires(internal_property::ValidOperator(Op)) struct AbelianGroup {
  using chalk_implies = void(Group<Op>, Commutative<Op>, ModuleOver<int>);
};

struct Ring {
  using chalk_implies = void(AbelianGroup<'+'>, Monoid<'*'>,
                             Distributes<'*', '+'>);
};

// Returns a constant reference to a a unit for `Op` over the structure `T`
// which has static storage duration.
template <char Op, Satisfies<HasIdentity<Op>> T>
requires(internal_property::ValidOperator(Op)) T const &Unit() {
  // TODO: Turn off destructor.
  static T unit = [] {
    if constexpr (Op == '+') {
      if constexpr (requires { T::Zero(); }) {
        return T::Zero();
      } else if constexpr (requires { T(0); }) {
        return T(0);
      }
    } else {
      if constexpr (requires { T::One(); }) {
        return T::One();
      } else if constexpr (requires { T(1); }) {
        return T(1);
      }
    }
  }();
  return unit;
}

namespace internal_property {

template <char C>
struct UnitImpl {
  template <Satisfies<HasIdentity<C>> T>
  operator T const &() const {
    return Unit<C, T>();
  }
};

}  // namespace internal_property

template <char Op>
requires(internal_property::ValidOperator(Op)) auto Unit() {
  return internal_property::UnitImpl<Op>();
}

// Returns a bool indicating whether `t` is the unit for `Op` in the structure
// `T`. By default, does so by comparing `t` to `Unit<Op, T>()`, but this
// behavior may be overridden with the `ChalkIsUnit` extension.
//
// TODO: Design and implement `ChalkIsUnit`.
template <char Op, int &..., typename T>
requires(Satisfies<T, HasIdentity<Op>>) bool IsUnit(T const &t) {
  return t == Unit<Op, T>();
}

// An empty base class for that algebraic structures must inherit from in order
// to have access to generated operators.
struct Algebraic {
  template <typename L, std::convertible_to<L> R>
  friend L operator+(L lhs, R &&rhs) requires(Satisfies<L, HasBinary<'+'>>) {
    lhs += std::forward<R>(rhs);
    return lhs;
  }

  template <typename R, std::convertible_to<R> L>
  friend R operator+(L &&lhs, R rhs) requires(
      not std::is_same_v<std::decay_t<L>, R> and
      Satisfies<R, HasBinary<'+'>, Commutative<'+'>>) {
    rhs += std::forward<L>(lhs);
    return rhs;
  }

  template <typename L, std::convertible_to<L> R>
  friend L operator*(L lhs, R &&rhs) requires(Satisfies<L, HasBinary<'*'>>) {
    lhs *= std::forward<R>(rhs);
    return lhs;
  }

  template <typename R, std::convertible_to<R> L>
  friend R operator*(L &&lhs, R rhs) requires(
      not std::is_same_v<std::decay_t<L>, R> and
      Satisfies<R, HasBinary<'*'>, Commutative<'*'>>) {
    rhs *= std::forward<L>(lhs);
    return rhs;
  }

  template <typename L, Satisfies<ScalableBy<L>> R>
  friend R operator*(L &&lhs, R rhs) {
    rhs *= std::forward<L>(lhs);
    return rhs;
  }

  template <typename R, Satisfies<ScalableBy<R>> L>
  friend R operator*(L lhs, R &&rhs) {
    lhs *= std::forward<L>(rhs);
    return lhs;
  }

  template <typename L, std::convertible_to<L> R>
  friend bool operator!=(L const &lhs, R const &rhs) {
    return not(lhs == rhs);
  }
};

namespace internal_property {

// Specialization for builtin types so they can be used without wrappers.
template <typename T>
requires(std::is_arithmetic_v<T>) struct PropertiesOfImpl<T> {
  using type = decltype(TraverseProperties(static_cast<void (*)(Ring)>(nullptr),
                                           static_cast<void (*)()>(nullptr)));
};

}  // namespace internal_property

}  // namespace chalk

#endif  // CHALK_ALGEBRA_PROPERTY_H

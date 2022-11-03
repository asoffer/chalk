#ifndef CHALK_ALGEBRA_PROPERTY_H
#define CHALK_ALGEBRA_PROPERTY_H

#include <concepts>
#include <type_traits>
#include <utility>

namespace chalk {
namespace internal_property {

constexpr bool ValidOperator(char c) { return c == '+' or c == '*'; }

template <typename... Processed>
auto TraverseProperties(void (*)(), void (*)(Processed...)) {
  return static_cast<void (*)(Processed...)>(nullptr);
}

template <typename P, typename... Ps, typename... Processed>
auto TraverseProperties(void (*)(P, Ps...), void (*)(Processed...)) {
  if constexpr ((std::is_same_v<P, Processed> or ...)) {
    return TraverseProperties(static_cast<void (*)(Ps...)>(nullptr),
                              static_cast<void (*)(Processed...)>(nullptr));
  } else if constexpr (requires { typename P::chalk_implies; }) {
    return TraverseProperties(
        static_cast<typename P::chalk_implies *>(nullptr),
        TraverseProperties(static_cast<void (*)(Ps...)>(nullptr),
                           static_cast<void (*)(P, Processed...)>(nullptr)));
  } else {
    return TraverseProperties(static_cast<void (*)(Ps...)>(nullptr),
                              static_cast<void (*)(P, Processed...)>(nullptr));
  }
}

template <typename T>
using PropertiesOf = decltype(TraverseProperties(
    static_cast<typename T::chalk_properties *>(nullptr),
    static_cast<void (*)()>(nullptr)));

template <typename, typename>
struct Contains;
template <typename... Ps, typename P>
struct Contains<void (*)(Ps...), P> {
  static constexpr bool value = (std::is_same_v<P, Ps> or ...);
};

}  // namespace internal_property

// Matches any structure whose properties (immediate and implied) match all of
// the `RequiredProperties`.
template <typename T, typename... RequiredProperties>
concept PropertiesInclude =
    (internal_property::Contains<internal_property::PropertiesOf<T>,
                                 RequiredProperties>::value and
     ...);

// Properties

// Indicates that the structure has a binary operator represented by `Op`.
template <char Op>
requires(internal_property::ValidOperator(Op)) struct HasBinary {
};

// Indicates that the structure has a commutative binary operator represented by
// `Op`.
template <char Op>
requires(internal_property::ValidOperator(Op)) struct Commutative {
  using chalk_implies = void(HasBinary<Op>);
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
requires(internal_property::ValidOperator(Op)) struct MonoidOver {
  using chalk_implies = void(HasBinary<'*'>, HasIdentity<'*'>);
};

// An empty base class for that algebraic structures must inherit from in order
// to have access to generated operators.
struct Algebraic {
  template <typename L, std::convertible_to<L> R>
  friend L operator+(L lhs,
                     R &&rhs) requires(PropertiesInclude<L, HasBinary<'+'>>) {
    lhs += std::forward<R>(rhs);
    return lhs;
  }

  template <typename R, std::convertible_to<R> L>
  friend R operator+(L &&lhs, R rhs) requires(
      not std::is_same_v<std::decay_t<L>, R> and
      PropertiesInclude<R, HasBinary<'+'>, Commutative<'+'>>) {
    rhs += std::forward<L>(lhs);
    return rhs;
  }

  template <typename L, std::convertible_to<L> R>
  friend L operator*(L lhs,
                     R &&rhs) requires(PropertiesInclude<L, HasBinary<'*'>>) {
    lhs *= std::forward<R>(rhs);
    return lhs;
  }

  template <typename R, std::convertible_to<R> L>
  friend R operator*(L &&lhs, R rhs) requires(
      not std::is_same_v<std::decay_t<L>, R> and
      PropertiesInclude<R, HasBinary<'*'>, Commutative<'*'>>) {
    rhs *= std::forward<L>(lhs);
    return rhs;
  }
};

}  // namespace chalk

#endif  // CHALK_ALGEBRA_PROPERTY_H

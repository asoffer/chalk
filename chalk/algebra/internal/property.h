#ifndef CHALK_ALGEBRA_INTERNAL_PROPERTY_H
#define CHALK_ALGEBRA_INTERNAL_PROPERTY_H

namespace chalk::internal_property {

// We use characters to represent operators, but only support '+' and '*'.
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

// Not to be specialized, other than in "chalk/properties.h" for primitive
// and/or standard-library types.
template <typename T>
struct PropertiesOfImpl;

template <typename T>
requires(requires {
  typename T::chalk_properties;
}) struct PropertiesOfImpl<T> {
  using type = decltype(TraverseProperties(
      static_cast<typename T::chalk_properties *>(nullptr),
      static_cast<void (*)()>(nullptr)));
};

template <typename T>
using PropertiesOf = typename PropertiesOfImpl<T>::type;

template <typename, typename>
struct Contains;
template <typename... Ps, typename P>
struct Contains<void (*)(Ps...), P> {
  static constexpr bool value = (std::is_same_v<P, Ps> or ...);
};

}  // namespace chalk::internal_property

#endif  // CHALK_ALGEBRA_INTERNAL_PROPERTY_H

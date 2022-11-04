#ifndef CHALK_BASE_TYPE_TRAITS_H
#define CHALK_BASE_TYPE_TRAITS_H

#include <type_traits>

namespace chalk::base {

template <typename Q, typename T>
using forward_type = std::conditional_t<
    std::is_lvalue_reference_v<Q>,
    std::conditional_t<std::is_const_v<std::remove_reference_t<Q>>, T const &,
                       T &>,
    T&&>;

}  // namespace chalk::base

#endif  // CHALK_BASE_TYPE_TRAITS_H

#include "chalk/base/type_traits.h"

#include <type_traits>

namespace chalk::base {

static_assert(std::is_same_v<forward_type<int, bool>, bool &&>);
static_assert(std::is_same_v<forward_type<int &, bool>, bool &>);
static_assert(std::is_same_v<forward_type<int const &, bool>, bool const &>);
static_assert(std::is_same_v<forward_type<int &&, bool>, bool &&>);
}  // namespace chalk::base

int main() { return 0; }

#ifndef CHALK_BASE_LAZY_H
#define CHALK_BASE_LAZY_H

#include <concepts>
#include <type_traits>

namespace chalk::base {
// TODO: This belongs somewhere more core/basic.
template <std::invocable F>
struct Lazy {
  Lazy(F f) : f_(std::move(f)) {}
  operator auto() const { return f_(); }

 private:
  F f_;
};

}  // namespace chalk::base

#endif  // CHALK_BASE_LAZY_H

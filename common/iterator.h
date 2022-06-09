#ifndef CHALK_ITERATOR_ITERATOR_H
#define CHALK_ITERATOR_ITERATOR_H

#include <type_traits>
#include <utility>

namespace chalk::common {

template <typename B, typename E>
struct iterator_range {
  using value_type     = std::decay_t<decltype(*std::declval<B>())>;
  using const_iterator = B;

  iterator_range(B b, E e) : begin_(std::move(b)), end_(std::move(e)) {}

  B begin() const { return begin_; }
  E end() const { return end_; }

  bool empty() const { return begin_ == end_; }

 private:
  [[no_unique_address]] B begin_;
  [[no_unique_address]] E end_;
};

template <typename B, typename E>
iterator_range(B, E) -> iterator_range<B, E>;

}  // namespace chalk::common

#endif  // CHALK_ITERATOR_ITERATOR_H

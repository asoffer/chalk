#ifndef CHALK_COMBINATORICS_SEQUENCE_H
#define CHALK_COMBINATORICS_SEQUENCE_H

#include <algorithm>
#include <utility>
#include <vector>

namespace chalk {

// Given a finite sequence `sequence` of values, returns a container consisting
// of the lengths of spaces between values that compare equal to `value`.
std::vector<size_t> GapsBetween(
    auto const& sequence,
    typename std::decay_t<decltype(sequence)>::value_type const& value) {
  std::vector<size_t> v;

  using ::std::begin;
  using ::std::end;
  if (begin(sequence) == end(sequence)) { return v; }

  size_t count = 0;
  for (auto const& element : sequence) {
    if (element == value) {
      v.push_back(std::exchange(count, 0));
    } else {
      ++count;
    }
  }
  v.push_back(count);
  return v;
}

}  // namespace chalk

#endif  // CHALK_COMBINATORICS_SEQUENCE_H

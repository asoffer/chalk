#ifndef CHALK_COMBINATORICS_COMPOSITION_H
#define CHALK_COMBINATORICS_COMPOSITION_H

#include <cassert>
#include <concepts>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace chalk {

// Represents an integer composition, where each part is stored as a `PartType`
// in contiguous storage.
template <std::integral PartType>
struct BasicComposition {
 private:
  using representation_type =
      std::conditional_t<std::is_standard_layout_v<PartType> and
                             std::is_trivial_v<PartType>,
                         std::basic_string<PartType>, std::vector<PartType>>;

 public:
  using value_type     = PartType;
  using const_iterator = typename representation_type::const_iterator;

  // Constructs the unique composition of 0.
  BasicComposition() = default;

  // Constructs a composition from the given parts.
  BasicComposition(std::initializer_list<value_type> parts)
      : parts_(parts.begin(), parts.end()) {}

  template <typename Iter>
  explicit BasicComposition(Iter b, Iter e) : parts_(b, e) {}

  // Returns the number of parts in the partition.
  constexpr size_t parts() const { return parts_.size(); }

  // Returns the integer `n` for which `*this` is a partition.
  constexpr size_t whole() const {
    size_t total = 0;
    for (size_t n : parts_) { total += n; }
    return total;
  }

  auto begin() { return parts_.begin(); }
  auto begin() const { return parts_.begin(); }
  auto cbegin() const { return parts_.cbegin(); }
  auto rbegin() { return parts_.rbegin(); }
  auto rbegin() const { return parts_.rbegin(); }
  auto crbegin() const { return parts_.crbegin(); }
  auto end() { return parts_.end(); }
  auto end() const { return parts_.end(); }
  auto cend() const { return parts_.cend(); }
  auto rend() { return parts_.rend(); }
  auto rend() const { return parts_.rend(); }
  auto crend() const { return parts_.crend(); }

  value_type &operator[](size_t i) {
    assert(i < parts_.size());
    return parts_[i];
  }
  value_type const &operator[](size_t i) const {
    assert(i < parts_.size());
    return parts_[i];
  }

  friend bool operator==(BasicComposition const &lhs,
                         BasicComposition const &rhs) {
    return lhs.parts_ == rhs.parts_;
  }

  friend bool operator!=(BasicComposition const &lhs,
                         BasicComposition const &rhs) {
    return not(lhs == rhs);
  }

  void append(value_type v) { parts_.push_back(v); }

  template <typename H>
  friend H AbslHashValue(H h, BasicComposition const &p) {
    return H::combine_contiguous(std::move(h), &*p.parts_.begin(), p.parts());
  }

  friend std::ostream &operator<<(std::ostream &os, BasicComposition const &p) {
    std::string_view separator = "(";
    for (auto part : p.parts_) {
      os << std::exchange(separator, ", ") << uint64_t{part};
    }
    return os << ")";
  }

  constexpr representation_type &&extract() && { return std::move(parts_); }

 protected:
  explicit BasicComposition(representation_type parts)
      : parts_(std::move(parts)) {}

  representation_type parts_;
};

using Composition = BasicComposition<uint8_t>;

}  // namespace chalk

#endif  // CHALK_COMBINATORICS_COMPOSITION_H

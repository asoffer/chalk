#ifndef CHALK_COMBINATORICS_DYCK_PATH_H
#define CHALK_COMBINATORICS_DYCK_PATH_H

#include <cassert>
#include <concepts>
#include <vector>

#include "chalk/combinatorics/composition.h"
#include "chalk/combinatorics/image.h"

namespace chalk {

// Represents an Dyck Path, a path from (0, 0) to (2n, 0) consisting of steps in
// the direction (+1, +1) and (+1, -1) that never go below the x-axis.
struct DyckPath {
  enum class Step : bool { Up, Down };

  using value_type = Step;

  DyckPath() = default;

  DyckPath(std::initializer_list<Step> steps) {
    implementation_.reserve(steps.size());
    for (Step step : steps) {
      implementation_.push_back(static_cast<bool>(step));
    }
  }

  // Returns a `DyckPath` consisting of `height` up steps followed by `height`
  // down steps.
  static DyckPath Peak(size_t height);

  // Returns a `DyckPath` consisting of `peaks` up/down pairs.
  static DyckPath Minimal(size_t peaks);

  // Moves the first peak into the first valley following that peak.
  void topple();

  // Returns the length of the path in terms of the number of steps taken
  // (always an even number).
  constexpr size_t size() const { return implementation_.size(); }

  // Returns whether the path is empty (i.e., has size zero).
  constexpr bool empty() const { return implementation_.empty(); }

  // Returns the direction of the `n`th step (up or down). Requires that `n <
  // this->size()`.
  Step operator[](size_t n) const {
    assert(n < size());
    return static_cast<Step>(static_cast<bool>(implementation_[n]));
  }

  // Returns a container containing all `DyckPath`s consisting of `n` upsteps
  // and `n` downsteps.
  static std::vector<DyckPath> All(size_t n);

  // Returns the `DyckPath` constructed by concatenating `*this` and `rhs
  // together.
  DyckPath &operator+=(DyckPath const &rhs);

  friend DyckPath operator+(DyckPath lhs, DyckPath const &rhs) {
    return lhs += rhs;
  }

  // Prepends the `DyckPath` with `height` up-steps and appends `height`
  // down-steps.
  DyckPath &lift(size_t height = 1);

  // Returns the `DyckPath` constructed from `p` by first having an upstem, then
  // the path `p`, then a final downstep.
  static DyckPath Lifted(DyckPath const &p, size_t height = 1);

  bool operator==(DyckPath const &lhs) const = default;
  bool operator!=(DyckPath const &lhs) const = default;

  template <typename H>
  friend H AbslHashValue(H h, DyckPath const &p) {
    return H::combine(std::move(h), p.implementation_);
  }

  struct const_iterator : private std::vector<bool>::const_iterator {
   private:
    using base = std::vector<bool>::const_iterator;

   public:
    using value_type = Step;

    const_iterator() = default;
    explicit const_iterator(base b) : base(b) {}

    auto &operator++() {
      ++static_cast<base &>(*this);
      return *this;
    }
    const_iterator operator++(int) {
      auto copy = *this;
      static_cast<base &>(*this)++;
      return copy;
    }
    auto &operator--() {
      --static_cast<base &>(*this);
      return *this;
    }
    const_iterator operator--(int) {
      auto copy = *this;
      static_cast<base &>(*this)--;
      return copy;
    }
    auto &operator+=(int rhs) {
      static_cast<base &>(*this) += rhs;
      return *this;
    }
    auto &operator-=(int rhs) {
      static_cast<base &>(*this) -= rhs;
      return *this;
    }

    friend auto operator+(const_iterator lhs, int rhs) { return lhs += rhs; }
    friend auto operator+(int lhs, const_iterator rhs) { return rhs += lhs; }
    friend auto operator-(const_iterator lhs, int rhs) { return lhs -= rhs; }
    friend auto operator-(const_iterator lhs, const_iterator rhs) {
      return static_cast<base>(lhs) - static_cast<base>(rhs);
    }

    Step operator*() const {
      return static_cast<Step>(static_cast<bool>(*static_cast<base>(*this)));
    }

    friend bool operator==(const_iterator lhs, const_iterator rhs) {
      return static_cast<base>(lhs) == static_cast<base>(rhs);
    }
    friend bool operator!=(const_iterator lhs, const_iterator rhs) {
      return not(lhs == rhs);
    }
  };

  const_iterator begin() const {
    return const_iterator(implementation_.begin());
  }
  const_iterator end() const { return const_iterator(implementation_.end()); }

  friend Image ChalkVisualize(DyckPath const &path);

 private:
  std::vector<bool> implementation_;
};

// Returns the area under `path`. That is, the number of full unit squares
// (rotated 45 degrees) which fit underneath the path and above x-axis. There is
// exactly one path with area zero (the path alternating between up and down
// steps).
size_t Area(DyckPath const &path);

// Returns the "bounce" statistic for `path` defined in
// https://www.sciencedirect.com/science/article/pii/S0195669804000320
size_t Bounce(DyckPath const &path);

// Returns the lengths of each section of the "bounce path" as a `Composition`
// associated to `path` as defined in
// https://www.sciencedirect.com/science/article/pii/S0195669804000320
Composition BouncePath(DyckPath const &path);

}  // namespace chalk

#endif  // CHALK_COMBINATORICS_DYCK_PATH_H

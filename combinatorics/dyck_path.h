#ifndef CHALK_COMBINATORICS_DYCK_PATH_H
#define CHALK_COMBINATORICS_DYCK_PATH_H

#include <cassert>
#include <ostream>
#include <vector>

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

  // Returns the length of the path in terms of the number of steps taken
  // (always an even number).
  constexpr size_t size() const { return implementation_.size(); }

  // Returns whether the path is empty (i.e., has size zero).
  constexpr bool empty() const { return implementation_.empty(); }

  // Returns the direction of the `n`th step (up or down). Requires that `n <
  // this->size()`.
  Step operator[](size_t n) const {
    assert(n < size());
    return static_cast<Step>(implementation_[n]);
  }

  // Returns a container containing all `DyckPath`s consisting of `n` upsteps
  // and `n` downsteps.
  static std::vector<DyckPath> All(size_t n);

  // Returns the `DyckPath` constructed by concatenating `lhs` and `rhs together.
  static DyckPath Concatenate(DyckPath lhs, DyckPath const &rhs);

  // Returns the `DyckPath` constructed from `p` by first having an upstem, then
  // the path `p`, then a final downstep.
  static DyckPath Lift(DyckPath const &p);

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
    const_iterator() = default;
    explicit const_iterator(base b) : base(b) {}

    auto &operator++() {
      ++static_cast<base &>(*this);
      return *this;
    }
    auto &operator++(int) {
      static_cast<base &>(*this)++;
      return *this;
    }
    auto &operator--() {
      --static_cast<base &>(*this);
      return *this;
    }
    auto &operator--(int) {
      static_cast<base &>(*this)--;
      return *this;
    }
    auto &operator+=(int rhs) {
      static_cast<base&>(*this) += rhs;
      return *this;
    }
    auto &operator-=(int rhs) {
      static_cast<base&>(*this) -= rhs;
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

  friend std::ostream &operator<<(std::ostream &os, DyckPath const &p) {
    os << "DyckPath[";
    for (DyckPath::Step step : p) {
      os << (step == DyckPath::Step::Up ? "(" : ")");
    }
    return os << "]";
  }

 private:
  std::vector<bool> implementation_;
};

// Returns the area under `path`. That is, the number of full unit squares
// (rotated 45 degrees) which fit underneath the path and above x-axis. There is
// exactly one path with area zero (the path alternating between up and down
// steps).
size_t Area(DyckPath const& path);

// Returns the "bounce" statistic for `path` defined in
// https://www.sciencedirect.com/science/article/pii/S0195669804000320
size_t Bounce(DyckPath const& path);

}  // namespace chalk

#endif  // CHALK_COMBINATORICS_DYCK_PATH_H
